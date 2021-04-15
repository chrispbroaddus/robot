#pragma once

#include "box_geometry.h"

namespace dense_mapping {
namespace insert_policies {

    /// Insert policy that is suitable for tracking explicit free space. This will enter every that intersects with a
    /// line segment.
    ///
    /// Note: this policy ends up visiting a lot more nodes during most traversals; as a result, it will tend to be
    /// significantly slower. If you don't need explicit modeling of empty space, it is probably better to just
    template <typename T> struct TraverseSegmentInsertPolicy {
        static constexpr bool shouldEnter(const std::array<T, 3>& volumeCenter, T volumeHalfExtent, const std::array<T, 3>& origin,
            const std::array<T, 3>& /* target */, const std::array<T, 3>& reciprocalDirection) {
            return Geometry::segmentIntersectsAxisAlignedBoundingBox(volumeHalfExtent, volumeCenter, origin, reciprocalDirection);
        }
    };

    /// Insert policy that is suitable for tracking only occupied space. This is much faster because it only visits leaves
    /// which intersect the source point.
    template <typename T> struct TraverseBoxInsertPolicy {

        static constexpr bool shouldEnter(const std::array<T, 3>& volumeCenter, T volumeHalfExtent, const std::array<T, 3>& /* origin */,
            const std::array<T, 3>& target, const std::array<T, 3>& /* reciprocalDirection */) {
            return Geometry::pointIntersectsAxisAlignedBoundingBox(target, volumeCenter, volumeHalfExtent);
        }
    };
}
}