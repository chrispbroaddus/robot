
#pragma once

#include "Eigen/Dense"

namespace imu_propagator {
namespace details {
    /// IMU samples in the propagator have a time and 3d array for gyroscope and accelerometer data.
    /// \tparam TIMESTAMP precision of the timestamp (float, double)
    /// \tparam SCALAR precision of the IMU data
    template <typename TIMESTAMP, typename SCALAR> class ImuSample {
    public:
        typedef Eigen::Matrix<SCALAR, 3, 1> gyro_type;
        typedef Eigen::Matrix<SCALAR, 3, 1> accel_type;

        ImuSample()
            : m_timestamp(0)
            , m_gyro(gyro_type(0, 0, 0))
            , m_accel(accel_type(0, 0, 0)) {}
        ImuSample(const TIMESTAMP timestamp, const gyro_type& gyro, const accel_type& accel)
            : m_timestamp(timestamp)
            , m_gyro(gyro)
            , m_accel(accel) {}
        ~ImuSample() = default;

        const TIMESTAMP timestamp() const { return m_timestamp; }
        const gyro_type& gyro() const { return m_gyro; }
        const accel_type& accel() const { return m_accel; }

    private:
        const TIMESTAMP m_timestamp;
        const gyro_type m_gyro;
        const accel_type m_accel;
    };
}
}
