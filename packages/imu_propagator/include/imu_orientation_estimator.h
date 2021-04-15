
#pragma once

#include "Eigen/Geometry"
#include "packages/imu_propagator/include/imu_database_details.h"
#include <stdexcept>

namespace imu_propagator {

/// Implements a complementary filter for orientation estimation.
/// World is defined as: (x,y) plane is parallel to ground-plane, z is pointing up.
template <typename SCALAR> class ImuOrientationEstimator {
public:
    using imu_sample = details::ImuSample<double, SCALAR>;

    /// Construct an orientation estimator from an initial quaternion.
    /// \param time Timestamp of initial quaternion
    /// \param initialQuaternion Intitial quaternion
    /// \param alpha Value of 0 weights accelerometer, and value of 1 weights the gyro
    ImuOrientationEstimator(const double time, const Eigen::Quaternion<SCALAR>& initialQuaternion, const SCALAR alpha)
        : m_time(time)
        , m_quaternion(initialQuaternion)
        , m_alpha(alpha) {
        if (alpha < 0 || alpha > 1) {
            throw std::out_of_range("alpha out of range");
        }
    }
    ~ImuOrientationEstimator() = default;

    /// Updates the filter state.
    /// \param sample IMU sample
    void update(const imu_sample& sample) {
        const double dt = sample.timestamp() - m_time;
        if (dt <= 0) {
            throw std::runtime_error("timestamps are not increasing");
        }

        // This operation could be improved with the expoential map, i.e. R = expm(ssm(w))
        const Eigen::Matrix<SCALAR, 3, 1> w(sample.gyro()(0) * dt, sample.gyro()(1) * dt, sample.gyro()(2) * dt);
        const Eigen::AngleAxis<SCALAR> deltaGyro(w.norm(), w);

        // Integrate gyroscope
        m_quaternion = m_quaternion * deltaGyro;

        // Compute a normalize vector with accelerometer components
        Eigen::Matrix<SCALAR, 3, 1> v(sample.accel()(0), sample.accel()(1), sample.accel()(2));
        v.normalize();

        // Let (x,y) plane be parallel to the ground plane and z point up.
        // The axis of rotation is orthogonal to the (z,v) plane.
        // n = cross(v, [0,0,1])
        // dot(v, [0,0,1]) = cos(phi), thus phi = acos(vz)
        const Eigen::Matrix<SCALAR, 3, 1> n(v(1), -v(0), 0);
        const SCALAR phi = std::acos(v(2));
        const Eigen::AngleAxis<SCALAR> accelCorrection((1 - m_alpha) * phi, n.normalized());

        m_quaternion = accelCorrection * m_quaternion;

        m_time = sample.timestamp();
    }

    /// \return Orientation from body to world
    const Eigen::Quaternion<SCALAR>& quaternion() const { return m_quaternion; }

private:
    double m_time;
    Eigen::Quaternion<SCALAR> m_quaternion;
    const SCALAR m_alpha;
};
}
