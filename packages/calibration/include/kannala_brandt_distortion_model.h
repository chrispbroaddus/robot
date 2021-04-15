#pragma once

#include "Eigen/Dense"
#include <limits>

namespace calibration {

template <typename T> class KannalaBrandtRadialDistortionModel4 {
public:
    KannalaBrandtRadialDistortionModel4(
        const Eigen::Matrix<T, 3, 3>& K, const Eigen::Matrix<T, 4, 1>& coef, const size_t numNewtonIterations, const T stoppingThreshold)
        : m_K(K)
        , m_coef(coef)
        , m_numNewtonIterations(numNewtonIterations)
        , m_stoppingThreshold(stoppingThreshold) {}
    ~KannalaBrandtRadialDistortionModel4() = default;

    /// Project a ray into a distored point.
    /// \param ray Source ray
    /// \param K Calibration matrix
    /// \param coef KB4 coefficients
    /// \return 2D image point
    Eigen::Matrix<T, 2, 1> project(const Eigen::Matrix<T, 3, 1>& ray) const {
        Eigen::Matrix<T, 3, 1> mh;
        const T mag = std::sqrt(ray(0) * ray(0) + ray(1) * ray(1));
        if (mag <= std::numeric_limits<T>::epsilon()) {
            mh(0) = 0;
            mh(1) = 0;
            mh(2) = 1;
        } else {
            // Equation 6
            const T theta = std::atan2(mag, ray(2));
            const T psi = std::atan2(ray(1), ray(0));
            const T theta2 = theta * theta;
            const T theta3 = theta2 * theta;
            const T theta5 = theta3 * theta2;
            const T theta7 = theta5 * theta2;
            const T theta9 = theta7 * theta2;
            const T rTheta = 1 * theta + m_coef(0) * theta3 + m_coef(1) * theta5 + m_coef(2) * theta7 + m_coef(3) * theta9;

            mh(0) = rTheta * std::cos(psi);
            mh(1) = rTheta * std::sin(psi);
            mh(2) = 1;
        }

        return (m_K * mh).hnormalized();
    }

    /// Unproject a distorted point into a ray.
    /// \param m Source 2D image point
    /// \param K Calibration matrix
    /// \param coef KB4 coefficients
    /// \return Unprojected ray
    Eigen::Matrix<T, 3, 1> unproject(const Eigen::Matrix<T, 2, 1>& m) const {
        Eigen::Matrix<T, 3, 1> ray;
        const Eigen::Matrix<T, 2, 1> mp = (m_K.inverse() * m.homogeneous()).hnormalized();
        const T rTheta = std::sqrt(mp(0) * mp(0) + mp(1) * mp(1));
        if (rTheta <= std::numeric_limits<T>::epsilon()) {
            ray(0) = 0;
            ray(1) = 0;
            ray(2) = 1;
        } else {
            const T psi = std::atan2(mp(1), mp(0));

            T theta = rTheta;
            for (size_t i = 0; i < m_numNewtonIterations; i++) {
                const T theta2 = theta * theta;
                const T theta3 = theta2 * theta;
                const T theta5 = theta3 * theta2;
                const T theta7 = theta5 * theta2;
                const T theta9 = theta7 * theta2;
                const T fx = 1 * theta + m_coef(0) * theta3 + m_coef(1) * theta5 + m_coef(2) * theta7 + m_coef(3) * theta9 - rTheta;
                const T df_dx
                    = 1 + theta * (3 * m_coef(0) * theta + 5 * m_coef(1) * theta3 + 7 * m_coef(2) * theta5 + 9 * m_coef(3) * theta7);
                const T delta = fx / df_dx;
                theta = theta - delta;
                if (std::abs(delta) <= m_stoppingThreshold) {
                    break;
                }
            }
            const T sinTheta = std::sin(theta);
            const T cosTheta = std::cos(theta);
            ray(0) = sinTheta * std::cos(psi) / cosTheta;
            ray(1) = sinTheta * std::sin(psi) / cosTheta;
            ray(2) = 1;
        }

        return ray;
    };

private:
    const Eigen::Matrix<T, 3, 3> m_K;
    const Eigen::Matrix<T, 4, 1> m_coef;
    const size_t m_numNewtonIterations;
    const T m_stoppingThreshold;
};
}
