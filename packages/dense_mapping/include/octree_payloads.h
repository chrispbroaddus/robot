#pragma once

#include "box_geometry.h"

namespace dense_mapping {
namespace payloads {

    /// All the types in this namespace use CRTP to be able to manipulate fields in the derived type but without
    /// dealing with any of the storage details. This gives us many of the benefits of inheritence (common code) without
    /// any of the potential costs (in this case, the ability to control the layout of derived types and make sure we
    /// remain fast)
    namespace mixins {
        /// Increment occupied count
        template <typename T, template <typename> class DERIVED_TEMPLATE> struct OccupiedCountMixin {
            template <typename... ARGS>
            static void updateImpl(DERIVED_TEMPLATE<T>& item, const std::array<T, 3>& /* volumeCenter */, T /* volumeHalfExtent */,
                const std::array<T, 3>& /* origin */, const std::array<T, 3>& /* target */, ARGS... /* unused */) {
                ++item.m_occupiedCount;
            }

            static void mergeImpl(DERIVED_TEMPLATE<T>& result, const DERIVED_TEMPLATE<T>& a, const DERIVED_TEMPLATE<T>& b) {
                result.m_occupiedCount = a.m_occupiedCount + b.m_occupiedCount;
            }
        };

        /// Update spatial m_moments
        ///
        /// Assumes OccupiedCountMixin has already run
        template <typename T, template <typename> class DERIVED_TEMPLATE> struct SpatialMomentsMixin {
            template <typename... ARGS>
            static void updateImpl(DERIVED_TEMPLATE<T>& item, const std::array<T, 3>& /* volumeCenter */, T /* volumeHalfExtent */,
                const std::array<T, 3>& /* origin */, const std::array<T, 3>& target, ARGS... /* args */) {
                const auto dx = target[0] - item.m_centroidX;
                const auto dy = target[1] - item.m_centroidY;
                const auto dz = target[2] - item.m_centroidZ;

                item.m_centroidX += dx / item.m_occupiedCount;
                item.m_centroidY += dy / item.m_occupiedCount;
                item.m_centroidZ += dz / item.m_occupiedCount;

                const auto dx2 = target[0] - item.m_centroidX;
                const auto dy2 = target[1] - item.m_centroidY;
                const auto dz2 = target[2] - item.m_centroidZ;

                item.m_momentXX += dx * dx2;
                item.m_momentXY += dx * dy2;
                item.m_momentXZ += dx * dz2;
                item.m_momentYY += dy * dy2;
                item.m_momentYZ += dy * dz2;
                item.m_momentZZ += dz * dz2;
            }

            static void mergeImpl(DERIVED_TEMPLATE<T>& result, const DERIVED_TEMPLATE<T>& a, const DERIVED_TEMPLATE<T>& b) {
                if (0 == a.m_occupiedCount) {
                    result.m_centroidX = b.m_centroidX;
                    result.m_centroidY = b.m_centroidY;
                    result.m_centroidZ = b.m_centroidZ;
                    result.m_momentXX = b.m_momentXX;
                    result.m_momentXY = b.m_momentXY;
                    result.m_momentXZ = b.m_momentXZ;
                    result.m_momentYY = b.m_momentYY;
                    result.m_momentYZ = b.m_momentYZ;
                    result.m_momentZZ = b.m_momentZZ;
                } else if (0 == b.m_occupiedCount) {
                    result.m_centroidX = a.m_centroidX;
                    result.m_centroidY = a.m_centroidY;
                    result.m_centroidZ = a.m_centroidZ;
                    result.m_momentXX = a.m_momentXX;
                    result.m_momentXY = a.m_momentXY;
                    result.m_momentXZ = a.m_momentXZ;
                    result.m_momentYY = a.m_momentYY;
                    result.m_momentYZ = a.m_momentYZ;
                    result.m_momentZZ = a.m_momentZZ;
                } else {
                    const auto dx = a.m_centroidX - b.m_centroidX;
                    const auto dy = a.m_centroidY - b.m_centroidY;
                    const auto dz = a.m_centroidZ - b.m_centroidZ;
                    const auto total = a.m_occupiedCount + b.m_occupiedCount;

                    // Per https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Parallel_algorithm,
                    // in expectation the occupied counts are approximately equal and potentially large, and the means
                    // are approximately equal so we expect potentially catastrophic cancellation and, hence, prefer this
                    // approach for updating means
                    result.m_centroidX = (a.m_occupiedCount * a.m_centroidX + b.m_occupiedCount * b.m_centroidX) / total;
                    result.m_centroidY = (a.m_occupiedCount * a.m_centroidY + b.m_occupiedCount * b.m_centroidY) / total;
                    result.m_centroidZ = (a.m_occupiedCount * a.m_centroidZ + b.m_occupiedCount * b.m_centroidZ) / total;

                    // 3.1 in http://prod.sandia.gov/techlib/access-control.cgi/2008/086212.pdf
                    result.m_momentXX = a.m_momentXX + b.m_momentXX + dx * dx * a.m_occupiedCount * b.m_occupiedCount / total;
                    result.m_momentXY = a.m_momentXY + b.m_momentXY + dx * dy * a.m_occupiedCount * b.m_occupiedCount / total;
                    result.m_momentXZ = a.m_momentXZ + b.m_momentXZ + dx * dz * a.m_occupiedCount * b.m_occupiedCount / total;
                    result.m_momentYY = a.m_momentYY + b.m_momentYY + dy * dy * a.m_occupiedCount * b.m_occupiedCount / total;
                    result.m_momentYZ = a.m_momentYZ + b.m_momentYZ + dy * dz * a.m_occupiedCount * b.m_occupiedCount / total;
                    result.m_momentZZ = a.m_momentZZ + b.m_momentZZ + dz * dz * a.m_occupiedCount * b.m_occupiedCount / total;
                }
            }
        };

        /// Update view direction
        ///
        /// Assumes OccupiedCountMixin has already run
        template <typename T, template <typename> class DERIVED_TEMPLATE> struct ViewDirectionMixin {
            template <typename... ARGS>
            static void updateImpl(DERIVED_TEMPLATE<T>& item, const std::array<T, 3>& /* volumeCenter */, T /* volumeHalfExtent */,
                const std::array<T, 3>& origin, const std::array<T, 3>& target, ARGS... /* args */) {
                const auto viewVector = Geometry::normalizedDirectionVector(origin, target);
                item.m_averageViewDirectionX += (viewVector[0] - item.m_averageViewDirectionX) / item.m_occupiedCount;
                item.m_averageViewDirectionY += (viewVector[1] - item.m_averageViewDirectionY) / item.m_occupiedCount;
                item.m_averageViewDirectionZ += (viewVector[2] - item.m_averageViewDirectionZ) / item.m_occupiedCount;
            }

            static void mergeImpl(DERIVED_TEMPLATE<T>& result, const DERIVED_TEMPLATE<T>& a, const DERIVED_TEMPLATE<T>& b) {
                if (0 == a.m_occupiedCount) {
                    result.m_averageViewDirectionX = b.m_averageViewDirectionX;
                    result.m_averageViewDirectionY = b.m_averageViewDirectionY;
                    result.m_averageViewDirectionZ = b.m_averageViewDirectionZ;
                } else if (0 == b.m_occupiedCount) {
                    result.m_averageViewDirectionX = a.m_averageViewDirectionX;
                    result.m_averageViewDirectionY = a.m_averageViewDirectionY;
                    result.m_averageViewDirectionZ = a.m_averageViewDirectionZ;
                } else {
                    const auto total = a.m_occupiedCount + b.m_occupiedCount;

                    // Per https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Parallel_algorithm,
                    // in expectation the occupied counts are approximately equal and potentially large, and the means
                    // are approximately equal so we expect potentially catastrophic cancellation and, hence, prefer this
                    // approach for updating means
                    result.m_averageViewDirectionX
                        = (a.m_occupiedCount * a.m_averageViewDirectionX + b.m_occupiedCount * b.m_averageViewDirectionX) / total;
                    result.m_averageViewDirectionY
                        = (a.m_occupiedCount * a.m_averageViewDirectionY + b.m_occupiedCount * b.m_averageViewDirectionY) / total;
                    result.m_averageViewDirectionZ
                        = (a.m_occupiedCount * a.m_averageViewDirectionZ + b.m_occupiedCount * b.m_averageViewDirectionZ) / total;
                }
            }
        };

        /// Convenience that does three things:
        /// 1. Ensure that DERIVED_TEMPLATE inherits from all the requested mixins
        /// 2. Provide an implementation of updateImpl which delegates to all the composed mixins from left to right
        /// 3. Provide an updateWorker template method which is a thin wrapper around updateImpl (syntactic sugar)
        template <typename T, template <typename> class DERIVED_TEMPLATE, template <typename, template <typename> class> class MIXIN,
            template <typename, template <typename> class> class... REST>
        struct CompositeUpdateMixin : public MIXIN<T, DERIVED_TEMPLATE>, public REST<T, DERIVED_TEMPLATE>... {
            template <typename... ARGS>
            static void updateImpl(DERIVED_TEMPLATE<T>& item, const std::array<T, 3>& volumeCenter, T volumeHalfExtent,
                const std::array<T, 3>& origin, const std::array<T, 3>& target, ARGS... args) {
                MIXIN<T, DERIVED_TEMPLATE>::template updateImpl(
                    item, volumeCenter, volumeHalfExtent, origin, target, std::forward<ARGS>(args)...);
                CompositeUpdateMixin<T, DERIVED_TEMPLATE, REST...>::template updateImpl(
                    item, volumeCenter, volumeHalfExtent, origin, target, std::forward<ARGS>(args)...);
            }

            template <typename... ARGS>
            void updateWorker(const std::array<T, 3>& volumeCenter, T volumeHalfExtent, const std::array<T, 3>& origin,
                const std::array<T, 3>& target, ARGS... args) {
                updateImpl(
                    *static_cast<DERIVED_TEMPLATE<T>*>(this), volumeCenter, volumeHalfExtent, origin, target, std::forward<ARGS>(args)...);
            }

            static void mergeImpl(DERIVED_TEMPLATE<T>& result, const DERIVED_TEMPLATE<T>& a, const DERIVED_TEMPLATE<T>& b) {
                MIXIN<T, DERIVED_TEMPLATE>::mergeImpl(result, a, b);
                CompositeUpdateMixin<T, DERIVED_TEMPLATE, REST...>::mergeImpl(result, a, b);
            }

            static void mergeWorker(DERIVED_TEMPLATE<T>& result, const DERIVED_TEMPLATE<T>& a, const DERIVED_TEMPLATE<T>& b) {
                mergeImpl(result, a, b);
            }
        };

        /// Base case for recursion: do nothing.
        template <typename T, template <typename> class DERIVED_TEMPLATE, template <typename, template <typename> class> class MIXIN>
        struct CompositeUpdateMixin<T, DERIVED_TEMPLATE, MIXIN> {
            template <typename... ARGS>
            static void updateImpl(DERIVED_TEMPLATE<T>& item, const std::array<T, 3>& volumeCenter, T volumeHalfExtent,
                const std::array<T, 3>& origin, const std::array<T, 3>& target, ARGS... args) {
                MIXIN<T, DERIVED_TEMPLATE>::template updateImpl(
                    item, volumeCenter, volumeHalfExtent, origin, target, std::forward<ARGS>(args)...);
            }

            static void mergeImpl(DERIVED_TEMPLATE<T>& result, const DERIVED_TEMPLATE<T>& a, const DERIVED_TEMPLATE<T>& b) {
                MIXIN<T, DERIVED_TEMPLATE>::mergeImpl(result, a, b);
            }
        };
    }

    template <typename T>
    struct ExplicitFreeSpaceOctreeLeafNode : public mixins::CompositeUpdateMixin<T, ExplicitFreeSpaceOctreeLeafNode,
                                                 mixins::OccupiedCountMixin, mixins::SpatialMomentsMixin, mixins::ViewDirectionMixin> {

        constexpr ExplicitFreeSpaceOctreeLeafNode()
            : m_sortKey(0)
            , m_emptyCount(0)
            , m_occupiedCount(0)
            , m_centroidX(0)
            , m_centroidY(0)
            , m_centroidZ(0)
            , m_momentXX(0)
            , m_momentXY(0)
            , m_momentXZ(0)
            , m_momentYY(0)
            , m_momentYZ(0)
            , m_momentZZ(0)
            , m_averageViewDirectionX(0)
            , m_averageViewDirectionY(0)
            , m_averageViewDirectionZ(0) {}

        /// Sort key
        uint64_t m_sortKey;

        /// How many times a ray/segment has passed through this volume
        uint64_t m_emptyCount;

        /// How many times a ray/segment has terminated in this volume
        uint64_t m_occupiedCount;

        /// Average of all the points which are contained within this volume
        T m_centroidX;
        T m_centroidY;
        T m_centroidZ;

        /// Spatial m_moments (un-normalized/unscaled)
        T m_momentXX;
        T m_momentXY;
        T m_momentXZ;
        T m_momentYY;
        T m_momentYZ;
        T m_momentZZ;

        /// Average view direction
        T m_averageViewDirectionX;
        T m_averageViewDirectionY;
        T m_averageViewDirectionZ;

        void update(
            const std::array<T, 3>& volumeCenter, T volumeHalfExtent, const std::array<T, 3>& origin, const std::array<T, 3>& target) {

            if (Geometry::pointIntersectsAxisAlignedBoundingBox(target, volumeCenter, volumeHalfExtent)) {
                this->updateWorker(volumeCenter, volumeHalfExtent, origin, target);
            } else {
                ++m_emptyCount;
            }
        }

        static void merge(ExplicitFreeSpaceOctreeLeafNode<T>& result, const ExplicitFreeSpaceOctreeLeafNode<T>& a,
            const ExplicitFreeSpaceOctreeLeafNode<T>& b) {
            a.mergeWorker(result, a, b);
            result.m_emptyCount = a.m_emptyCount + b.m_emptyCount;
        }

        ExplicitFreeSpaceOctreeLeafNode<T> merge(const ExplicitFreeSpaceOctreeLeafNode<T>& other) const {
            ExplicitFreeSpaceOctreeLeafNode<T> result;
            result.m_sortKey = m_sortKey;
            merge(result, *this, other);
            return result;
        }

        ExplicitFreeSpaceOctreeLeafNode<T>& mergeInPlace(const ExplicitFreeSpaceOctreeLeafNode<T>& other) {
            *this = merge(other);
            return *this;
        }
    };

    template <typename T>
    struct OccupiedOnlyOctreeLeafNode
        : public mixins::CompositeUpdateMixin<T, OccupiedOnlyOctreeLeafNode, mixins::OccupiedCountMixin, mixins::SpatialMomentsMixin> {

        constexpr OccupiedOnlyOctreeLeafNode()
            : m_sortKey(0)
            , m_occupiedCount(0)
            , m_centroidX(0)
            , m_centroidY(0)
            , m_centroidZ(0)
            , m_momentXX(0)
            , m_momentXY(0)
            , m_momentXZ(0)
            , m_momentYY(0)
            , m_momentYZ(0)
            , m_momentZZ(0) {}

        /// Sort key
        uint64_t m_sortKey;

        /// How many times a ray/segment has terminated in this volume
        uint64_t m_occupiedCount;

        /// Average of all the points which are contained within this volume
        T m_centroidX;
        T m_centroidY;
        T m_centroidZ;

        /// Spatial m_moments (un-normalized/unscaled)
        T m_momentXX;
        T m_momentXY;
        T m_momentXZ;
        T m_momentYY;
        T m_momentYZ;
        T m_momentZZ;

        void update(
            const std::array<T, 3>& volumeCenter, T volumeHalfExtent, const std::array<T, 3>& origin, const std::array<T, 3>& target) {
            this->updateWorker(volumeCenter, volumeHalfExtent, origin, target);
        }

        static void merge(
            OccupiedOnlyOctreeLeafNode<T>& result, const OccupiedOnlyOctreeLeafNode<T>& a, const OccupiedOnlyOctreeLeafNode<T>& b) {
            a.mergeWorker(result, a, b);
        }

        OccupiedOnlyOctreeLeafNode<T> merge(const OccupiedOnlyOctreeLeafNode<T>& other) const {
            OccupiedOnlyOctreeLeafNode<T> result;
            result.m_sortKey = m_sortKey;
            merge(result, *this, other);
            return result;
        }

        OccupiedOnlyOctreeLeafNode<T>& mergeInPlace(const OccupiedOnlyOctreeLeafNode<T>& other) {
            *this = merge(other);
            return *this;
        }
    };
}
}