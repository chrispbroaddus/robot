#pragma once

#include "thirdparty/Sophus/sophus/se3.hpp"

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace core {

template <typename T> class PoseInterpolator {
public:
    /// Delegating c'tor
    /// \tparam CONTAINER_TYPE something that satisfies
    /// \param x
    template <typename CONTAINER_TYPE>
    explicit PoseInterpolator(const CONTAINER_TYPE& x)
        : PoseInterpolator(std::cbegin(x), std::cend(x)) {}

    ///
    /// \tparam CONST_FORWARD_ITERATOR_TYPE
    /// \param begin
    /// \param end
    template <typename CONST_FORWARD_ITERATOR_TYPE>
    PoseInterpolator(CONST_FORWARD_ITERATOR_TYPE begin, CONST_FORWARD_ITERATOR_TYPE end)
        : m_poses(begin, end) {
        std::sort(m_poses.begin(), m_poses.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

        for (const auto& x : m_poses) {
            if (!std::isfinite(x.first)) {
                throw std::invalid_argument("Found at least one pose with non-finite time");
            }
        }

        for (size_t i = 1; i < m_poses.size(); ++i) {
            if (m_poses.at(i - 1).first == m_poses.at(i).first) {
                throw std::invalid_argument("Found duplicate time stamps -- should not be the case");
            }
        }
    }

    /// Estimate an interpolated pose at the specified time.
    ///
    /// -# If the pose set backing this interpolator is empty, throw -- there is nothing we can do.
    /// -# If requested time occurs before the first time in the pose set or after the last time in the pose set, throw
    /// -# If we have a pose which corresponds exactly to the requested time, return that
    /// -# Otherwise, interpolate between the newest pose prior to the requested time and the oldest pose after the
    ///    requested time, using SE3 geodesic (similar in spirit to SLERP)
    /// .
    /// \param time
    /// \return
    constexpr Sophus::SE3<T> interpolateAtTime(T time) const {
        if (!std::isfinite(time)) {
            throw std::invalid_argument("Requested time should be finite");
        }

        // Least element in m_poses which is greater than time
        const auto upper = std::upper_bound(
            m_poses.begin(), m_poses.end(), time, [](const T& a, const std::pair<T, Sophus::SE3<T> >& b) { return a < b.first; });

        if (upper == m_poses.end()) {
            throw std::invalid_argument("Requested time to interpolate is after the last element in the interval");
        }

        if (upper == m_poses.begin()) {
            throw std::invalid_argument("Requested time to interpolate is before the first element in the interval");
        }

        // Greatest element in m_poses which is <= time. We know this is safe because the two assertions above can only
        // be satisfied if there are at least two items in m_poses.
        const auto lower = std::prev(upper);

        if (lower->first == time) {
            // Found an exact match
            return lower->second;
        } else {
            // both of these are guaranteed strictly positive due to invariant in the constructor (unique, finite, sorted)
            // and above (dealing with exact equality)
            const auto deltaNumerator = time - lower->first;
            const auto deltaDenominator = upper->first - lower->first;

            if (deltaNumerator <= 0) {
                throw std::logic_error("Invariant that time > lower->first violated");
            }

            if (deltaDenominator <= 0) {
                throw std::logic_error("Invariant that poses are stored internally sorted ascending by time violated");
            }

            if (deltaNumerator > deltaDenominator) {
                throw std::logic_error("Invariant that time is bracketed by lower and upper violated");
            }

            // Can guarantee (TM) that this is strictly in the range (0, 1)
            const auto scale = deltaNumerator / deltaDenominator;
            return Sophus::SE3<T>::exp(scale * ((upper->second * lower->second.inverse()).log()));
        }
    }

private:
    std::vector<std::pair<T, Sophus::SE3<T> > > m_poses;
};
}