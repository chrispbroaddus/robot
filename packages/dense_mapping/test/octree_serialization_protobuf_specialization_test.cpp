#include "../include/octree_serialization.h"
#include "../include/insert_policies.h"
#include "../include/octree.h"
#include "../include/octree_payloads.h"
#include "../include/octree_serialization_specializations.h"

#include "gtest/gtest.h"

namespace {
template <typename T> class OctreeSerializeToProtobufTest : public ::testing::Test {};

using TypesToTest = ::testing::Types<float, double>;

TYPED_TEST_CASE(OctreeSerializeToProtobufTest, TypesToTest);

TYPED_TEST(OctreeSerializeToProtobufTest, checkExplicitFreeSpacePayload) {
    using scalar_type = TypeParam;
    using leaf_node_type = dense_mapping::payloads::ExplicitFreeSpaceOctreeLeafNode<scalar_type>;
    using serialized_type = dense_mapping::VolumetricModel;
    using helper_type = dense_mapping::serialization::details::SerializationHelper<scalar_type,
        dense_mapping::payloads::ExplicitFreeSpaceOctreeLeafNode, serialized_type>;
    leaf_node_type node;
    node.m_sortKey = 1;
    node.m_occupiedCount = 2;
    node.m_emptyCount = 3;
    node.m_centroidX = 4;
    node.m_centroidY = 5;
    node.m_centroidZ = 6;
    node.m_momentXX = 7;
    node.m_momentXY = 8;
    node.m_momentXZ = 9;
    node.m_momentYY = 10;
    node.m_momentYZ = 11;
    node.m_momentZZ = 12;
    node.m_averageViewDirectionX = 13;
    node.m_averageViewDirectionY = 14;
    node.m_averageViewDirectionZ = 15;

    serialized_type target;
    helper_type::serialize(target, node, std::array<scalar_type, 3>{ { 16, 17, 18 } }, 19);

    ASSERT_EQ(1, target.voxels_size());
    const auto& vox = target.voxels(0);
    EXPECT_EQ(16, vox.voxel_center_x());
    EXPECT_EQ(17, vox.voxel_center_y());
    EXPECT_EQ(18, vox.voxel_center_z());
    EXPECT_EQ(node.m_occupiedCount, vox.occupancy_count());
    EXPECT_EQ(node.m_emptyCount, vox.empty_space().empty_space_count());
    EXPECT_EQ(node.m_centroidX, vox.spatial_moments().centroid_x());
    EXPECT_EQ(node.m_centroidY, vox.spatial_moments().centroid_y());
    EXPECT_EQ(node.m_centroidZ, vox.spatial_moments().centroid_z());
    EXPECT_EQ(node.m_momentXX, vox.spatial_moments().moment_xx());
    EXPECT_EQ(node.m_momentXY, vox.spatial_moments().moment_xy());
    EXPECT_EQ(node.m_momentXZ, vox.spatial_moments().moment_xz());
    EXPECT_EQ(node.m_momentYY, vox.spatial_moments().moment_yy());
    EXPECT_EQ(node.m_momentYZ, vox.spatial_moments().moment_yz());
    EXPECT_EQ(node.m_momentZZ, vox.spatial_moments().moment_zz());
    EXPECT_EQ(node.m_averageViewDirectionX, vox.average_view_direction().x());
    EXPECT_EQ(node.m_averageViewDirectionY, vox.average_view_direction().y());
    EXPECT_EQ(node.m_averageViewDirectionZ, vox.average_view_direction().z());
}

TYPED_TEST(OctreeSerializeToProtobufTest, checkOccupiedOnlySpacePayload) {
    using scalar_type = TypeParam;
    using leaf_node_type = dense_mapping::payloads::OccupiedOnlyOctreeLeafNode<scalar_type>;
    using serialized_type = dense_mapping::VolumetricModel;
    using helper_type = dense_mapping::serialization::details::SerializationHelper<scalar_type,
        dense_mapping::payloads::OccupiedOnlyOctreeLeafNode, serialized_type>;
    leaf_node_type node;
    node.m_sortKey = 1;
    node.m_occupiedCount = 2;
    node.m_centroidX = 3;
    node.m_centroidY = 4;
    node.m_centroidZ = 5;
    node.m_momentXX = 6;
    node.m_momentXY = 7;
    node.m_momentXZ = 8;
    node.m_momentYY = 9;
    node.m_momentYZ = 10;
    node.m_momentZZ = 11;

    serialized_type target;
    helper_type::serialize(target, node, std::array<scalar_type, 3>{ { 12, 13, 14 } }, 15);

    ASSERT_EQ(1, target.voxels_size());
    const auto& vox = target.voxels(0);
    EXPECT_EQ(12, vox.voxel_center_x());
    EXPECT_EQ(13, vox.voxel_center_y());
    EXPECT_EQ(14, vox.voxel_center_z());
    EXPECT_EQ(node.m_occupiedCount, vox.occupancy_count());
    EXPECT_EQ(node.m_centroidX, vox.spatial_moments().centroid_x());
    EXPECT_EQ(node.m_centroidY, vox.spatial_moments().centroid_y());
    EXPECT_EQ(node.m_centroidZ, vox.spatial_moments().centroid_z());
    EXPECT_EQ(node.m_momentXX, vox.spatial_moments().moment_xx());
    EXPECT_EQ(node.m_momentXY, vox.spatial_moments().moment_xy());
    EXPECT_EQ(node.m_momentXZ, vox.spatial_moments().moment_xz());
    EXPECT_EQ(node.m_momentYY, vox.spatial_moments().moment_yy());
    EXPECT_EQ(node.m_momentYZ, vox.spatial_moments().moment_yz());
    EXPECT_EQ(node.m_momentZZ, vox.spatial_moments().moment_zz());
}
}