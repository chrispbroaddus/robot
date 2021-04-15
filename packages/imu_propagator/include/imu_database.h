
#pragma once

#include "packages/imu_propagator/include/imu_database_details.h"

#include <algorithm>
#include <iostream>
#include <list>
#include <stdexcept>

namespace imu_propagator {

/// IMU samples in the propagator have a time and 3d array for gyroscope and accelerometer data.
/// \tparam TIMESTAMP precision of the timestamp (float, double)
/// \tparam SCALAR precision of the IMU data
template <typename TIMESTAMP, typename SCALAR> class ImuDatabase {
public:
    typedef details::ImuSample<TIMESTAMP, SCALAR> imu_sample_type;
    typedef std::list<imu_sample_type> list_type;
    typedef std::tuple<imu_sample_type, imu_sample_type> imu_sample_pair_type;

    ///
    /// \param maxSize maximum size of the IMU database
    ImuDatabase(const size_t maxSize)
        : m_maxSize(maxSize) {}
    ~ImuDatabase() = default;

    /// Add an IMU sample to the database. The database size will be kept <= to the max size.
    /// \param sample The sample to add
    void addImuSample(const imu_sample_type& sample) {
        if (!m_samples.empty() && (sample.timestamp() <= m_samples.back().timestamp())) {
            throw std::runtime_error("timestamps decreasing or duplicated");
        }

        if (m_samples.size() == m_maxSize) {
            m_samples.pop_front();
        }
        m_samples.push_back(sample);
    }

    /// Get the lower and upper bound such that t0 <= timestamp < t1.
    /// \param timestamp
    /// \return pair of IMU samples
    imu_sample_pair_type adjacentSamplesAtTime(const TIMESTAMP timestamp) const {
        typename list_type::const_iterator upperBoundIter = std::upper_bound(m_samples.begin(), m_samples.end(), timestamp,
            [](const TIMESTAMP value, const imu_sample_type& sample) { return sample.timestamp() > value; });

        if (upperBoundIter == m_samples.end()) {
            throw std::domain_error("std::upper_bound() failed");
        }

        typename list_type::const_iterator leftIter = std::prev(upperBoundIter);
        if (leftIter == m_samples.end()) {
            throw std::domain_error("no imu sample to left of timestamp");
        }

        const auto imu_sample_pair = std::make_tuple(*leftIter, *upperBoundIter);

        return imu_sample_pair;
    }

    /// Get all the IMU samples in a timestsamp range [t0,t1].
    /// \param range timestamp range
    /// \return list of IMU samples
    list_type inRange(const std::tuple<TIMESTAMP, TIMESTAMP>& range) const {
        if (std::get<0>(range) == std::get<1>(range)) {
            throw std::domain_error("duplicate time values");
        }

        typename list_type::const_iterator leftLowerBoundIter = std::lower_bound(m_samples.begin(), m_samples.end(), std::get<0>(range),
            [](const imu_sample_type& sample, const TIMESTAMP value) { return sample.timestamp() < value; });

        if (leftLowerBoundIter == m_samples.end()) {
            throw std::domain_error("std::lower_bound() failed for t0");
        }

        list_type data;
        for (typename list_type::const_iterator it = leftLowerBoundIter; it != m_samples.end(); it++) {
            if (it->timestamp() >= std::get<0>(range) && it->timestamp() <= std::get<1>(range)) {
                data.push_back(*it);
            } else {
                break;
            }
        }

        return data;
    }

    /// Get a string representation of the IMU database.
    /// \return string
    const std::string toString() const {
        std::string str;

        std::ostringstream oss;
        oss << "maxSize: " << m_maxSize << std::endl;
        oss << "size: " << m_samples.size() << std::endl;
        for (const auto& sample : m_samples) {
            oss << "time: " << sample.timestamp() << " gyro:" << sample.gyro().transpose() << " accel: " << sample.accel().transpose()
                << std::endl;
        }

        return oss.str();
    }

private:
    const size_t m_maxSize;
    list_type m_samples;
};

/// Linear IMU interpolator for gyroscope and accelerometer.
/// \tparam TIMESTAMP precision of the timestamp (float, double)
/// \tparam SCALAR precision of the IMU data
template <typename TIMESTAMP, typename SCALAR> class LinearImuInterpolator {
public:
    typedef ImuDatabase<TIMESTAMP, SCALAR> imu_database_type;
    using imu_sample_type = typename imu_database_type::imu_sample_type;
    using imu_sample_pair_type = typename imu_database_type::imu_sample_pair_type;

    LinearImuInterpolator(const imu_database_type& imuDatabase)
        : m_imuDatabase(imuDatabase) {}

    /// Get IMU samples at a specific time.
    /// \param timestamp the requested timestamp
    /// \return interpolated IMU sample
    imu_sample_type getImuSampleAtTime(const TIMESTAMP timestamp) {
        const auto imuSamplePair = m_imuDatabase.adjacentSamplesAtTime(timestamp);

        const auto dt = std::get<1>(imuSamplePair).timestamp() - std::get<0>(imuSamplePair).timestamp();
        if (dt <= 0) {
            throw std::runtime_error("timestamps out of order or duplicated");
        }

        const auto t0 = timestamp - std::get<0>(imuSamplePair).timestamp();
        const auto t1 = std::get<1>(imuSamplePair).timestamp() - timestamp;

        const auto w0 = 1 - t0 / dt;
        const auto w1 = 1 - t1 / dt;

        const auto gyro = w0 * std::get<0>(imuSamplePair).gyro() + w1 * std::get<1>(imuSamplePair).gyro();
        const auto accel = w0 * std::get<0>(imuSamplePair).accel() + w1 * std::get<1>(imuSamplePair).accel();

        return imu_sample_type(timestamp, gyro, accel);
    }

private:
    const imu_database_type& m_imuDatabase;
};
}