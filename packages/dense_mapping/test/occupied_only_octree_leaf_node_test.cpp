#include "packages/dense_mapping/include/octree_payloads.h"

#include "gtest/gtest.h"

namespace {
template <typename T> struct OccupiedOnlyOctreeLeafNodeTest : public ::testing::Test {};

using TypesToTest = ::testing::Types<float, double>;

TYPED_TEST_CASE(OccupiedOnlyOctreeLeafNodeTest, TypesToTest);

TYPED_TEST(OccupiedOnlyOctreeLeafNodeTest, canInstantiate) { dense_mapping::payloads::OccupiedOnlyOctreeLeafNode<TypeParam> impl; }

TYPED_TEST(OccupiedOnlyOctreeLeafNodeTest, checkUpdateSingleItem) {
    dense_mapping::payloads::OccupiedOnlyOctreeLeafNode<TypeParam> impl;

    EXPECT_EQ(0, impl.m_sortKey);
    EXPECT_EQ(0, impl.m_occupiedCount);
    EXPECT_EQ(0, impl.m_centroidX);
    EXPECT_EQ(0, impl.m_centroidY);
    EXPECT_EQ(0, impl.m_centroidZ);
    EXPECT_EQ(0, impl.m_momentXX);
    EXPECT_EQ(0, impl.m_momentXY);
    EXPECT_EQ(0, impl.m_momentXZ);
    EXPECT_EQ(0, impl.m_momentYY);
    EXPECT_EQ(0, impl.m_momentYZ);
    EXPECT_EQ(0, impl.m_momentZZ);

    constexpr std::array<TypeParam, 3> center{ { 0, 0, 0 } };
    constexpr std::array<TypeParam, 3> source{ { 0, 0, 0 } };
    constexpr std::array<TypeParam, 3> target{ { 1, 1, 1 } };

    impl.update(center, 0, source, target);

    EXPECT_EQ(0, impl.m_sortKey);
    EXPECT_EQ(1, impl.m_occupiedCount);
    EXPECT_EQ(1, impl.m_centroidX);
    EXPECT_EQ(1, impl.m_centroidY);
    EXPECT_EQ(1, impl.m_centroidZ);
    EXPECT_EQ(0, impl.m_momentXX);
    EXPECT_EQ(0, impl.m_momentXY);
    EXPECT_EQ(0, impl.m_momentXZ);
    EXPECT_EQ(0, impl.m_momentYY);
    EXPECT_EQ(0, impl.m_momentYZ);
    EXPECT_EQ(0, impl.m_momentZZ);
}
}
