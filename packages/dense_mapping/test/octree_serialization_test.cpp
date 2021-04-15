#include "../include/octree.h"
#include "../include/insert_policies.h"
#include "../include/octree_payloads.h"
#include "../include/octree_serialization.h"

#include "gtest/gtest.h"

namespace {
struct DummySerializeTarget {
    DummySerializeTarget()
        : m_serializeHeaderCount(0)
        , m_visitedNodes() {}

    size_t m_serializeHeaderCount;
    std::vector<uint64_t> m_visitedNodes;
};
}

namespace dense_mapping {
namespace serialization {
    namespace details {
        template <typename SCALAR_TYPE, template <typename> class LEAF_NODE_TEMPLATE>
        struct SerializationHelper<SCALAR_TYPE, LEAF_NODE_TEMPLATE, DummySerializeTarget> {
            static void serialize(DummySerializeTarget& destination, const LEAF_NODE_TEMPLATE<SCALAR_TYPE>& leaf,
                const std::array<SCALAR_TYPE, 3>& /* voxelCenter*/, SCALAR_TYPE /*voxelHalfExtent*/) {
                destination.m_visitedNodes.push_back(leaf.m_sortKey);
            }

            template <template <typename> class INSERTION_POLICY>
            static void serialize(const Octree<SCALAR_TYPE, LEAF_NODE_TEMPLATE, INSERTION_POLICY>&, DummySerializeTarget& destination) {
                ++destination.m_serializeHeaderCount;
            }
        };
    }
}
}

namespace {
template <typename T> class OctreeSerializationTest : public ::testing::Test {};

namespace dm = dense_mapping;
namespace dmi = dense_mapping::insert_policies;
namespace dmp = dense_mapping::payloads;
namespace dms = dense_mapping::serialization;

using TypesToTest = ::testing::Types<dm::Octree<float, dmp::OccupiedOnlyOctreeLeafNode, dmi::TraverseBoxInsertPolicy>,
    dm::Octree<double, dmp::OccupiedOnlyOctreeLeafNode, dmi::TraverseBoxInsertPolicy>,
    dm::Octree<float, dmp::ExplicitFreeSpaceOctreeLeafNode, dmi::TraverseSegmentInsertPolicy>,
    dm::Octree<double, dmp::ExplicitFreeSpaceOctreeLeafNode, dmi::TraverseSegmentInsertPolicy> >;

TYPED_TEST_CASE(OctreeSerializationTest, TypesToTest);

TYPED_TEST(OctreeSerializationTest, canSerializeEmptyTree) {
    constexpr typename TypeParam::scalar_type halfExtent = 1;
    constexpr uint32_t maximumDepth = 4;
    TypeParam tree(halfExtent, maximumDepth);
    DummySerializeTarget target;
    dense_mapping::serialize(tree, target);

    EXPECT_EQ(1, target.m_serializeHeaderCount);
    EXPECT_TRUE(target.m_visitedNodes.empty());
}

TYPED_TEST(OctreeSerializationTest, canSerializeTree) {
    using octree_type = TypeParam;
    using scalar_type = typename octree_type::scalar_type;
    using point_type = typename octree_type::point_type;

    constexpr scalar_type halfExtent = 1;
    constexpr uint32_t maximumDepth = 4;
    TypeParam tree(halfExtent, maximumDepth);

    constexpr point_type originPoint{ { 0, 0, 0 } };
    constexpr point_type targetPoint(originPoint);

    tree.insert(originPoint, targetPoint);

    DummySerializeTarget target;
    dense_mapping::serialize(tree, target);

    EXPECT_EQ(1, target.m_serializeHeaderCount);
    EXPECT_FALSE(target.m_visitedNodes.empty());
    std::vector<uint64_t> unique(target.m_visitedNodes.begin(), target.m_visitedNodes.end());
    std::sort(unique.begin(), unique.end());
    unique.erase(std::unique(unique.begin(), unique.end()), unique.end());
    EXPECT_EQ(target.m_visitedNodes.size(), unique.size());
}
}