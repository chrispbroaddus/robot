#include "../include/box_geometry.h"
#include "gtest/gtest.h"

namespace {
template <typename T> struct BoxBoxIntersectionTest : public ::testing::Test {};

using TypesToTest = ::testing::Types<float, double>;

TYPED_TEST_CASE(BoxBoxIntersectionTest, TypesToTest);

TYPED_TEST(BoxBoxIntersectionTest, boxesDisjointTopAndBottomFaces) {
    constexpr int minX = -10;
    constexpr int maxX = 10;
    constexpr int minY = -10;
    constexpr int maxY = 10;
    constexpr TypeParam boxHalfHeight = 1;
    constexpr TypeParam boxSeparation = 1;

    for (int x1 = minX; x1 <= maxX; ++x1) {
        for (int y1 = minY; y1 <= maxY; ++y1) {
            const std::array<TypeParam, 3> center1{ { static_cast<TypeParam>(x1), static_cast<TypeParam>(y1), 0 } };

            for (int x2 = x1; x2 <= maxX; ++x2) {
                for (int y2 = y1; y2 <= maxY; ++y2) {
                    const std::array<TypeParam, 3> center2{ { static_cast<TypeParam>(x2), static_cast<TypeParam>(y2),
                        2 * boxHalfHeight + boxSeparation } };

                    EXPECT_FALSE(
                        dense_mapping::Geometry::axisAlignedBoxIntersectsAxisAlignedBox(center1, boxHalfHeight, center2, boxHalfHeight));
                    EXPECT_FALSE(
                        dense_mapping::Geometry::axisAlignedBoxIntersectsAxisAlignedBox(center2, boxHalfHeight, center1, boxHalfHeight));
                }
            }
        }
    }
}

TYPED_TEST(BoxBoxIntersectionTest, boxesDisjointLeftAndRightFaces) {
    constexpr int minY = -10;
    constexpr int maxY = 10;
    constexpr int minZ = -10;
    constexpr int maxZ = 10;
    constexpr TypeParam boxHalfHeight = 1;
    constexpr TypeParam boxSeparation = 1;

    for (int y1 = minY; y1 <= maxY; ++y1) {
        for (int z1 = minZ; z1 <= maxZ; ++z1) {
            const std::array<TypeParam, 3> center1{ { 0, static_cast<TypeParam>(y1), static_cast<TypeParam>(z1) } };

            for (int y2 = y1; y2 <= maxY; ++y2) {
                for (int z2 = z1; z2 <= maxZ; ++z2) {
                    const std::array<TypeParam, 3> center2{ { 2 * boxHalfHeight + boxSeparation, static_cast<TypeParam>(y2),
                        static_cast<TypeParam>(z2) } };

                    EXPECT_FALSE(
                        dense_mapping::Geometry::axisAlignedBoxIntersectsAxisAlignedBox(center1, boxHalfHeight, center2, boxHalfHeight));
                    EXPECT_FALSE(
                        dense_mapping::Geometry::axisAlignedBoxIntersectsAxisAlignedBox(center2, boxHalfHeight, center1, boxHalfHeight));
                }
            }
        }
    }
}

TYPED_TEST(BoxBoxIntersectionTest, boxesDisjointBackFace) {
    constexpr int minX = -10;
    constexpr int maxX = 10;
    constexpr int minZ = -10;
    constexpr int maxZ = 10;
    constexpr TypeParam boxHalfHeight = 1;
    constexpr TypeParam boxSeparation = 1;

    for (int x1 = minX; x1 <= maxX; ++x1) {
        for (int z1 = minZ; z1 <= maxZ; ++z1) {
            const std::array<TypeParam, 3> center1{ { static_cast<TypeParam>(x1), 0, static_cast<TypeParam>(z1) } };

            for (int x2 = x1; x2 <= maxX; ++x2) {
                for (int z2 = z1; z2 <= maxZ; ++z2) {
                    const std::array<TypeParam, 3> center2{ { static_cast<TypeParam>(x2), 2 * boxHalfHeight + boxSeparation,
                        static_cast<TypeParam>(z2) } };

                    EXPECT_FALSE(
                        dense_mapping::Geometry::axisAlignedBoxIntersectsAxisAlignedBox(center1, boxHalfHeight, center2, boxHalfHeight));
                    EXPECT_FALSE(
                        dense_mapping::Geometry::axisAlignedBoxIntersectsAxisAlignedBox(center2, boxHalfHeight, center1, boxHalfHeight));
                }
            }
        }
    }
}

TYPED_TEST(BoxBoxIntersectionTest, boxesIntersectEdgeCases) {
    constexpr TypeParam halfExtent = 1;
    constexpr std::array<TypeParam, 3> center1{ { 0, 0, 0 } };

    // Consider all 26-connected neighbors
    for (int xOffset = -1; xOffset <= 1; ++xOffset) {
        const TypeParam x = xOffset * 2 * halfExtent;

        for (int yOffset = -1; yOffset <= 1; ++yOffset) {
            const TypeParam y = yOffset * 2 * halfExtent;

            for (int zOffset = -1; zOffset <= 1; ++zOffset) {
                const TypeParam z = zOffset * 2 * halfExtent;
                const std::array<TypeParam, 3> center2{ { x, y, z } };
                EXPECT_TRUE(dense_mapping::Geometry::axisAlignedBoxIntersectsAxisAlignedBox(center1, halfExtent, center2, halfExtent));
            }
        }
    }
}
}