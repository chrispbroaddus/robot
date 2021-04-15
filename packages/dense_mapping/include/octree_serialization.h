#pragma once

#include "octree.h"
#include "octree_queries.h"

namespace dense_mapping {
namespace serialization {
    namespace details {
        template <typename SCALAR_TYPE, template <typename> class LEAF_NODE_TEMPLATE, typename SERIALIZED_TYPE> struct SerializationHelper {
            static void serialize(SERIALIZED_TYPE& /*destination*/, const LEAF_NODE_TEMPLATE<SCALAR_TYPE>& /*leaf*/,
                const std::array<SCALAR_TYPE, 3>& /*voxelCenter*/, SCALAR_TYPE /*voxelHalfExtent*/) {}

            template <template <typename> class INSERTION_POLICY>
            static void serialize(
                const Octree<SCALAR_TYPE, LEAF_NODE_TEMPLATE, INSERTION_POLICY>& /* tree */, SERIALIZED_TYPE& /* destination */) {}
        };
    }
}

template <typename SCALAR_TYPE, template <typename> class LEAF_NODE_TEMPLATE, template <typename> class INSERTION_POLICY,
    typename SERIALIZED_TYPE>
void serialize(const Octree<SCALAR_TYPE, LEAF_NODE_TEMPLATE, INSERTION_POLICY>& tree, SERIALIZED_TYPE& out) {
    const queries::AxisAlignedBoxQuery<SCALAR_TYPE> query(std::array<SCALAR_TYPE, 3>{ { 0, 0, 0 } }, tree.volumeLength());
    serialization::details::SerializationHelper<SCALAR_TYPE, LEAF_NODE_TEMPLATE, SERIALIZED_TYPE>::serialize(tree, out);

    tree.query(query, [&out](const LEAF_NODE_TEMPLATE<SCALAR_TYPE>& leafNode, const std::array<SCALAR_TYPE, 3>& voxelCenter,
                          SCALAR_TYPE voxelHalfExtent) {
        serialization::details::SerializationHelper<SCALAR_TYPE, LEAF_NODE_TEMPLATE, SERIALIZED_TYPE>::template serialize(
            out, leafNode, voxelCenter, voxelHalfExtent);
    });
}
}