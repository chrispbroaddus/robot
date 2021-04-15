#pragma once

#include "box_geometry.h"

namespace dense_mapping {
namespace queries {
    template <typename T> struct AxisAlignedBoxQuery {
        constexpr AxisAlignedBoxQuery(const std::array<T, 3>& center, T halfExtent)
            : m_center(center)
            , m_halfExtent(halfExtent) {}

        const std::array<T, 3> m_center;
        const T m_halfExtent;

        constexpr bool shouldEnter(const std::array<T, 3>& voxelCenter, T voxelHalfExtent) const {
            return Geometry::axisAlignedBoxIntersectsAxisAlignedBox(m_center, m_halfExtent, voxelCenter, voxelHalfExtent);
        }
    };

    template <typename T> struct FrustumQuery {
        constexpr FrustumQuery(const Geometry::Frustum<T>& f)
            : m_frustum(f) {}

        constexpr FrustumQuery(
            const std::array<T, 3>& apex, const std::array<T, 3>& orientation, T verticalHalfAngle, T horizontalHalfAngle)
            : m_frustum(Geometry::frustum(apex, orientation, horizontalHalfAngle, verticalHalfAngle)) {}

        const Geometry::Frustum<T> m_frustum;

        constexpr bool shouldEnter(const std::array<T, 3>& voxelCenter, T voxelHalfExtent) const {
            return Geometry::frustumIntersectsAxisAlignedBoundingBox(m_frustum, voxelCenter, voxelHalfExtent);
        }
    };

    template <typename T> struct SegmentQuery {
        /// \param a first point on the @emph{segment}
        /// \param b second point on the @empy{segment}
        constexpr SegmentQuery(const std::array<T, 3>& a, const std::array<T, 3>& b)
            : m_origin(a)
            , m_reciprocalDirection(Geometry::reciprocalDirectionVector(a, b)) {}

        const std::array<T, 3> m_origin;
        const std::array<T, 3> m_reciprocalDirection;

        constexpr bool shouldEnter(const std::array<T, 3>& voxelCenter, T voxelHalfExtent) const {
            return Geometry::segmentIntersectsAxisAlignedBoundingBox(voxelHalfExtent, voxelCenter, m_origin, m_reciprocalDirection);
        }
    };

    template <typename T> struct RayQuery {
        /// \param a point on the @emph{ray}
        /// \param b point on the @empy{ray}
        constexpr RayQuery(const std::array<T, 3>& a, const std::array<T, 3>& b)
            : m_origin(a)
            , m_reciprocalDirection(Geometry::reciprocalDirectionVector(a, b)) {}

        const std::array<T, 3> m_origin;
        const std::array<T, 3> m_reciprocalDirection;

        constexpr bool shouldEnter(const std::array<T, 3>& voxelCenter, T voxelHalfExtent) const {
            return Geometry::rayIntersectsAxisAlignedBoundingBox(voxelHalfExtent, voxelCenter, m_origin, m_reciprocalDirection);
        }
    };
}
}