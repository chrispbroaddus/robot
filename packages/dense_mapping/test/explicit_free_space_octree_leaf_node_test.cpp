#include "packages/dense_mapping/include/octree_payloads.h"

#include "gtest/gtest.h"

namespace {
template <typename T> struct ExplicitFreeSpaceOctreeLeafNodeTest : public ::testing::Test {};

using TypesToTest = ::testing::Types<float, double>;

TYPED_TEST_CASE(ExplicitFreeSpaceOctreeLeafNodeTest, TypesToTest);

TYPED_TEST(ExplicitFreeSpaceOctreeLeafNodeTest, canInstantiate) {
    dense_mapping::payloads::ExplicitFreeSpaceOctreeLeafNode<TypeParam> impl;

    EXPECT_EQ(0, impl.m_sortKey);
    EXPECT_EQ(0, impl.m_occupiedCount);
    EXPECT_EQ(0, impl.m_emptyCount);
    EXPECT_EQ(0, impl.m_centroidX);
    EXPECT_EQ(0, impl.m_centroidY);
    EXPECT_EQ(0, impl.m_centroidZ);
    EXPECT_EQ(0, impl.m_momentXX);
    EXPECT_EQ(0, impl.m_momentXY);
    EXPECT_EQ(0, impl.m_momentXZ);
    EXPECT_EQ(0, impl.m_momentYY);
    EXPECT_EQ(0, impl.m_momentYZ);
    EXPECT_EQ(0, impl.m_momentZZ);
    EXPECT_EQ(0, impl.m_averageViewDirectionX);
    EXPECT_EQ(0, impl.m_averageViewDirectionY);
    EXPECT_EQ(0, impl.m_averageViewDirectionZ);
}

TYPED_TEST(ExplicitFreeSpaceOctreeLeafNodeTest, checkUpdateSingleItemTerminatesInNode) {
    dense_mapping::payloads::ExplicitFreeSpaceOctreeLeafNode<TypeParam> impl;

    constexpr TypeParam halfExtent = 1;
    constexpr std::array<TypeParam, 3> center{ { 0, 0, 0 } };
    constexpr std::array<TypeParam, 3> source{ { 0, 0, 0 } };
    constexpr std::array<TypeParam, 3> target{ { 1, 1, 1 } };

    impl.update(center, halfExtent, source, target);

    EXPECT_EQ(0, impl.m_sortKey);
    EXPECT_EQ(1, impl.m_occupiedCount);
    EXPECT_EQ(0, impl.m_emptyCount);
    EXPECT_EQ(1, impl.m_centroidX);
    EXPECT_EQ(1, impl.m_centroidY);
    EXPECT_EQ(1, impl.m_centroidZ);
    EXPECT_EQ(0, impl.m_momentXX);
    EXPECT_EQ(0, impl.m_momentXY);
    EXPECT_EQ(0, impl.m_momentXZ);
    EXPECT_EQ(0, impl.m_momentYY);
    EXPECT_EQ(0, impl.m_momentYZ);
    EXPECT_EQ(0, impl.m_momentZZ);

    const auto expectedDirection = 1 / std::sqrt(static_cast<TypeParam>(3));

    EXPECT_NEAR(expectedDirection, impl.m_averageViewDirectionX, 1e-5);
    EXPECT_NEAR(expectedDirection, impl.m_averageViewDirectionY, 1e-5);
    EXPECT_NEAR(expectedDirection, impl.m_averageViewDirectionZ, 1e-5);
}

TYPED_TEST(ExplicitFreeSpaceOctreeLeafNodeTest, checkUpdateSingleItemTerminatesOutsideNode) {
    dense_mapping::payloads::ExplicitFreeSpaceOctreeLeafNode<TypeParam> impl;

    constexpr TypeParam halfExtent = 0.5;

    EXPECT_EQ(0, impl.m_sortKey);
    EXPECT_EQ(0, impl.m_occupiedCount);
    EXPECT_EQ(0, impl.m_emptyCount);
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

    impl.update(center, halfExtent, source, target);

    EXPECT_EQ(0, impl.m_sortKey);
    EXPECT_EQ(0, impl.m_occupiedCount);
    EXPECT_EQ(1, impl.m_emptyCount);
    EXPECT_EQ(0, impl.m_centroidX);
    EXPECT_EQ(0, impl.m_centroidY);
    EXPECT_EQ(0, impl.m_centroidZ);
    EXPECT_EQ(0, impl.m_momentXX);
    EXPECT_EQ(0, impl.m_momentXY);
    EXPECT_EQ(0, impl.m_momentXZ);
    EXPECT_EQ(0, impl.m_momentYY);
    EXPECT_EQ(0, impl.m_momentYZ);
    EXPECT_EQ(0, impl.m_momentZZ);
    EXPECT_EQ(0, impl.m_averageViewDirectionX);
    EXPECT_EQ(0, impl.m_averageViewDirectionY);
    EXPECT_EQ(0, impl.m_averageViewDirectionZ);
}
}