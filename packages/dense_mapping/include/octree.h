#pragma once

#include "box_geometry.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <unordered_map>

namespace dense_mapping {
/// Basic dense map representation.
///
/// \see{https://zippy.quip.com/rsJBAsLhHtqv/How-the-Dense-Mapping-Toolbox-Works}
///
/// \tparam T Underlying scalar type (e.g. float or double) -- precision with which geometry, etc. is stored
/// \tparam LEAF_TEMPLATE A \emph{template} which models the payload concept. At the very least, this template must
/// provide the following "interface" / signature:
/// -# Default constructible
/// -# Copy constructible
/// -# Public member called m_sortKey which is at least a uint64_t
/// -# An update method (see dense_mapping::payloads::OccupiedOnlyOctreeNode for an example)
/// -# A merge method (see dense_mapping::payloads::OccupiedOnlyOctreeNode for an example)
/// .
/// \tparam INSERT_POLICY A \emph{template} which controls traversal of the tree during insertion of data. See
/// dense_mapping::insertion::TraverseBoxInsertPolicy for an example
///
template <typename T, template <typename> class LEAF_TEMPLATE, template <typename> class INSERT_POLICY> class Octree {
public:
    using scalar_type = T;
    using insertion_policy = INSERT_POLICY<T>;
    using leaf_node_type = LEAF_TEMPLATE<T>;
    using point_type = std::array<scalar_type, 3>;

    /// Construtor
    ///
    /// \param halfExtent Half the length of the side of the modeled volume. E.g: setting this to 10 would result in a
    /// 20 x 20 x 20 cube as the overall working volume.
    /// \param depth The maximum depth of the tree. This cannot be more than 16 and must be at least 1.
    Octree(T halfExtent, uint32_t depth)
        : m_volumeHalfExtent(halfExtent)
        , m_maximumDepth(depth)
        , m_maximumLeafNodes(1ULL << (3 * depth))
        , m_leafNodes()
        , m_idToIndex()
        , m_idToOccupancySketch() {

        if (depth > kMaximumAllowedDepth) {
            throw std::runtime_error("Requested an octree with depth greater than that which is currently allowed");
        }

        if (halfExtent <= 0) {
            throw std::runtime_error("Requested an octree with an invalid half-extent (must be non-negative)");
        }

        m_leafNodes.reserve(512 * 1024);
    }

    /// Perform a non-destructive merge of this model with other and return the result.
    /// \param other The octree to merge with. It is assumed to have the same coordinate frame as this model. It must
    /// also have the same maximum depth and extents.
    ///
    /// \return The union of these two trees.
    Octree<T, LEAF_TEMPLATE, INSERT_POLICY> merge(const Octree<T, LEAF_TEMPLATE, INSERT_POLICY>& other) const {
        if (m_volumeHalfExtent != other.m_volumeHalfExtent) {
            throw std::runtime_error("Cannot merge octrees with different half-extents");
        }

        if (m_maximumDepth != other.m_maximumDepth) {
            throw std::runtime_error("Cannot merge octrees with different maximum depths");
        }

        Octree<T, LEAF_TEMPLATE, INSERT_POLICY> result(m_volumeHalfExtent, m_maximumDepth);

        result.m_leafNodes = mergeNodes(other);
        result.m_idToOccupancySketch = mergeOccupancySketch(m_idToOccupancySketch, other.m_idToOccupancySketch);
        for (size_t i = 0; i < result.m_leafNodes.size(); ++i) {
            result.m_idToIndex.insert(std::make_pair(result.m_leafNodes[i].m_sortKey, i));
        }

        return result;
    }

    /// Insert a point into the octree. The signature of this method is partially determined by the signature of the
    /// update method on the underlying payload type.
    ///
    /// \tparam ARGS
    /// \param source The point from which we are observing
    /// \param target The point that was observed
    /// \param args Any additional arguments required by the payload
    /// \return
    template <typename... ARGS> uint64_t insert(const point_type& source, const point_type& target, ARGS&&... args) {
        const auto reciprocalDirection = Geometry::reciprocalDirectionVector(source, target);
        return insertImpl(
            kZero, m_volumeHalfExtent, m_maximumDepth, 0ULL, source, target, reciprocalDirection, std::forward<ARGS>(args)...);
    }

    /// Visit all leaves in the tree; intended for performing serialization and other traversals which do not care
    /// about the spatial attributes of a voxel
    /// \param visitor
    void visitAllLeaves(std::function<void(const leaf_node_type&)> visitor) const {
        std::for_each(m_leafNodes.begin(), m_leafNodes.end(), visitor);
    }

    /// Visit all voxels which satisfy the provided query
    /// \tparam QUERY See dense_mapping::queries::AxisAlignedBoxQuery for an example
    /// \param query The query to perform
    /// \param visitor A visitor which is invoked on each leaf voxel which intersects with the query geometry
    template <typename QUERY> void query(QUERY query, std::function<void(const leaf_node_type&, const point_type&, T)> visitor) const {
        queryImpl(kZero, m_volumeHalfExtent, m_maximumDepth, 0ULL, query, visitor);
    }

    /// Visit all voxels which satisfy the provided query
    /// \tparam QUERY See dense_mapping::queries::AxisAlignedBoxQuery for an example
    /// \param query The query to perform
    /// \param visitor A visitor which is invoked on each leaf voxel which intersects with the query geometry
    template <typename QUERY> void query(QUERY query, std::function<void(const leaf_node_type&)> visitor) const {
        queryImpl(kZero, m_volumeHalfExtent, m_maximumDepth, 0ULL, query,
            [&visitor](const leaf_node_type& leaf, const point_type&, T) { visitor(leaf); });
    }

    /// Return the half extent for this volume
    constexpr T volumeLength() const { return m_volumeHalfExtent; }

    /// Return the depth of the tree
    constexpr uint32_t leafDepth() const { return m_maximumDepth; }

    /// Return tne maximum number of leaf nodes
    constexpr uint64_t leafCapacity() const { return m_maximumLeafNodes; }

    /// Return the current leaf node count
    constexpr size_t leafCount() const { return m_leafNodes.size(); }

    /// Convert the internal addresses (payload.m_sortKey) to the point which is that voxels center
    constexpr point_type boxCenterFromIndex(uint64_t index) const { return boxCenterFromIndex(index, m_volumeHalfExtent); }

    /// Convert the internal addresses (payload.m_sortKey) to the point which is that voxels center
    static constexpr point_type boxCenterFromIndex(uint64_t index, T halfExtent) {
        point_type result{ { 0, 0, 0 } };

        int maximumDepth = -1;
        for (uint32_t i = 0; i < kMaximumAllowedDepth; ++i) {
            const auto nibble = (index >> (i * 4)) & 0xf;

            if (nibble) {
                if (nibble & 0x8) {
                    maximumDepth = i;
                } else {
                    throw std::runtime_error("Invalid input (expect high bit of each nibble to be 1 or for the nibble to be zero)");
                }
            } else {
                break;
            }
        }

        for (int i = maximumDepth; i >= 0; --i) {
            const auto nibble = (index >> (4 * i)) & 0xf;

            result[0] += ((nibble & 0x1) ? 1 : -1) * halfExtent / 2;
            result[1] += ((nibble & 0x2) ? 1 : -1) * halfExtent / 2;
            result[2] += ((nibble & 0x4) ? 1 : -1) * halfExtent / 2;
            halfExtent /= 2;
        }

        return result;
    }

private:
    const T m_volumeHalfExtent;
    const uint32_t m_maximumDepth;
    const uint64_t m_maximumLeafNodes;

    static constexpr point_type kZero{ { 0, 0, 0 } };

    /// 64 bits in the ids we use, 4 bits per level (1 for marker, 3 for octant) -- deepest tree we can represent the addresses of leaf
    /// nodes
    static constexpr uint32_t kMaximumAllowedDepth = 64 / 4;

    std::vector<leaf_node_type> m_leafNodes;
    std::unordered_map<uint64_t, size_t> m_idToIndex;
    std::unordered_map<uint64_t, uint8_t> m_idToOccupancySketch;

    template <typename QUERY>
    void queryImpl(const point_type& boxOrigin, const T boxHalfExtent, const uint32_t remainingDepth, uint64_t currentNodeIndex,
        QUERY query, std::function<void(const leaf_node_type&, const point_type&, T)> visitor) const {
        if (remainingDepth >= 1) {
            // Dealing with an interior node.
            const T childHalfExtent = boxHalfExtent / 2;

            // Pull up the sketch for this particular voxel
            const auto iter = m_idToOccupancySketch.find(currentNodeIndex);

            // If there is anything in this particular voxel...
            if (iter != m_idToOccupancySketch.end()) {
                // Consider all children...
                for (size_t child = 0; child < 8; ++child) {
                    // ... if we know that child has data in it
                    if (iter->second & (1 << child)) {
                        const point_type childOrigin{
                            { ((0 != (child & 1)) ? boxOrigin[0] + childHalfExtent : boxOrigin[0] - childHalfExtent),
                                ((0 != (child & 2)) ? boxOrigin[1] + childHalfExtent : boxOrigin[1] - childHalfExtent),
                                ((0 != (child & 4)) ? boxOrigin[2] + childHalfExtent : boxOrigin[2] - childHalfExtent) }
                        };

                        if (query.shouldEnter(childOrigin, childHalfExtent)) {
                            const auto childIndex = (currentNodeIndex << 4) | (0x8 | (child & 0x7));
                            queryImpl(childOrigin, childHalfExtent, remainingDepth - 1, childIndex, query, visitor);
                        }
                    }
                }
            }
        } else {
            auto iter = m_idToIndex.find(currentNodeIndex);

            if (iter != m_idToIndex.end()) {
                visitor(m_leafNodes.at(iter->second), boxOrigin, boxHalfExtent);
            }
        }
    }

    template <typename... ARGS>
    uint64_t insertImpl(const point_type& boxOrigin, const T boxHalfExtent, const uint32_t remainingDepth, const uint64_t currentNodeIndex,
        const point_type& source, const point_type& target, const point_type& reciprocalDirection, ARGS&&... args) {

        uint64_t traversedVoxels = 1;

        if (remainingDepth >= 1) {
            // Dealing with an interior node.
            const T childHalfExtent = boxHalfExtent / 2;

            // Consider all children
            for (size_t child = 0; child < 8; ++child) {
                const point_type childOrigin{ { ((0 != (child & 1)) ? boxOrigin[0] + childHalfExtent : boxOrigin[0] - childHalfExtent),
                    ((0 != (child & 2)) ? boxOrigin[1] + childHalfExtent : boxOrigin[1] - childHalfExtent),
                    ((0 != (child & 4)) ? boxOrigin[2] + childHalfExtent : boxOrigin[2] - childHalfExtent) } };

                if (insertion_policy::shouldEnter(boxOrigin, boxHalfExtent, source, target, reciprocalDirection)) {
                    auto iter = m_idToOccupancySketch.find(currentNodeIndex);

                    if (iter == m_idToOccupancySketch.end()) {
                        iter = m_idToOccupancySketch.insert(std::make_pair(currentNodeIndex, uint8_t(0))).first;
                    }

                    iter->second |= 1 << child;

                    const auto childIndex = (currentNodeIndex << 4) | (0x8 | (child & 0x7));
                    traversedVoxels += insertImpl(childOrigin, childHalfExtent, remainingDepth - 1, childIndex, source, target,
                        reciprocalDirection, std::forward<ARGS>(args)...);
                }
            }
        } else {
            // Dealing with a leaf node
            auto iter = m_idToIndex.find(currentNodeIndex);

            if (iter == m_idToIndex.end()) {
                const auto idx = m_leafNodes.size();
                m_leafNodes.push_back(leaf_node_type());
                m_leafNodes.back().m_sortKey = currentNodeIndex;
                iter = m_idToIndex.insert(std::make_pair(currentNodeIndex, idx)).first;
            }

            auto& node = m_leafNodes[iter->second];
            node.update(boxOrigin, boxHalfExtent, source, target, std::forward<ARGS>(args)...);
        }

        return traversedVoxels;
    }

    std::vector<leaf_node_type> mergeNodes(const Octree<T, LEAF_TEMPLATE, INSERT_POLICY>& other) const {
        std::vector<leaf_node_type> mergedNodes;

        for (const auto& node : m_idToIndex) {
            auto rhs = other.m_idToIndex.find(node.first);

            if (rhs == other.m_idToIndex.end()) {
                mergedNodes.push_back(m_leafNodes.at(node.second));
            } else {
                mergedNodes.push_back(m_leafNodes.at(node.second).merge(other.m_leafNodes.at(rhs->second)));
            }
        }

        for (const auto& rhs : other.m_idToIndex) {
            if (m_idToIndex.find(rhs.first) == m_idToIndex.end()) {
                mergedNodes.push_back(other.m_leafNodes.at(rhs.second));
            }
        }

        std::sort(mergedNodes.begin(), mergedNodes.end(),
            [](const leaf_node_type& a, const leaf_node_type& b) { return a.m_sortKey < b.m_sortKey; });

        return mergedNodes;
    }

    static std::unordered_map<uint64_t, uint8_t> mergeOccupancySketch(
        const std::unordered_map<uint64_t, uint8_t>& left, const std::unordered_map<uint64_t, uint8_t>& right) {

        std::unordered_map<uint64_t, uint8_t> result;

        for (const auto& sketch : left) {
            auto rhs = right.find(sketch.first);

            if (rhs != right.end()) {
                result.insert(std::make_pair(sketch.first, sketch.second | rhs->second));
            } else {
                result.insert(sketch);
            }
        }

        for (const auto& sketch : right) {
            if (left.end() == left.find(sketch.first)) {
                result.insert(sketch);
            }
        }

        return result;
    }
};

template <typename T, template <typename> class P, template <typename> class I>
constexpr typename Octree<T, P, I>::point_type Octree<T, P, I>::kZero;
template <typename T, template <typename> class P, template <typename> class I> constexpr uint32_t Octree<T, P, I>::kMaximumAllowedDepth;
}