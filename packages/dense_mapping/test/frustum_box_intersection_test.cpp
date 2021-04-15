#include "../include/box_geometry.h"

#include "gtest/gtest.h"

namespace {
template <typename T> class FrustumIntersectsAxisAlignedBoundingBoxTest : public ::testing::Test {};

using TypesToTest = ::testing::Types<float, double>;

TYPED_TEST_CASE(FrustumIntersectsAxisAlignedBoundingBoxTest, TypesToTest);

TYPED_TEST(FrustumIntersectsAxisAlignedBoundingBoxTest, detailsAllBoxCornersInSameHalfSpacePositiveSide) {
    constexpr TypeParam halfExtent = 1;
    constexpr std::array<TypeParam, 3> boxCenter{ { 0, 0, 0 } };
    constexpr std::array<TypeParam, 3> pointInPlane{ { 0, 0, -2 * halfExtent } };
    constexpr std::array<TypeParam, 3> planeNormal{ { 0, 0, 1 } };

    EXPECT_EQ(1, dense_mapping::Geometry::details::allCornersInSameHalfSpace(planeNormal, pointInPlane, boxCenter, halfExtent));
}

TYPED_TEST(FrustumIntersectsAxisAlignedBoundingBoxTest, detailsAllBoxCornersInSameHalfSpacePositiveSideTouching) {
    constexpr TypeParam halfExtent = 1;
    constexpr std::array<TypeParam, 3> boxCenter{ { 0, 0, 0 } };
    constexpr std::array<TypeParam, 3> pointInPlane{ { 0, 0, -halfExtent } };
    constexpr std::array<TypeParam, 3> planeNormal{ { 0, 0, 1 } };

    EXPECT_EQ(1, dense_mapping::Geometry::details::allCornersInSameHalfSpace(planeNormal, pointInPlane, boxCenter, halfExtent));
}

TYPED_TEST(FrustumIntersectsAxisAlignedBoundingBoxTest, detailsAllBoxCornersInSameHalfSpaceStraddling) {
    constexpr TypeParam halfExtent = 1;
    constexpr std::array<TypeParam, 3> boxCenter{ { 0, 0, 0 } };
    constexpr std::array<TypeParam, 3> pointInPlane{ { 0, 0, 0 } };
    constexpr std::array<TypeParam, 3> planeNormal{ { 0, 0, 1 } };

    EXPECT_EQ(0, dense_mapping::Geometry::details::allCornersInSameHalfSpace(planeNormal, pointInPlane, boxCenter, halfExtent));
}

TYPED_TEST(FrustumIntersectsAxisAlignedBoundingBoxTest, detailsAllBoxCornersInSameHalfSpaceNegativeSideTouching) {
    constexpr TypeParam halfExtent = 1;
    constexpr std::array<TypeParam, 3> boxCenter{ { 0, 0, 0 } };
    constexpr std::array<TypeParam, 3> pointInPlane{ { 0, 0, halfExtent } };
    constexpr std::array<TypeParam, 3> planeNormal{ { 0, 0, 1 } };

    EXPECT_EQ(-1, dense_mapping::Geometry::details::allCornersInSameHalfSpace(planeNormal, pointInPlane, boxCenter, halfExtent));
}

TYPED_TEST(FrustumIntersectsAxisAlignedBoundingBoxTest, detailsAllBoxCornersInSameHalfSpaceNegativeSide) {
    constexpr TypeParam halfExtent = 1;
    constexpr std::array<TypeParam, 3> boxCenter{ { 0, 0, 0 } };
    constexpr std::array<TypeParam, 3> pointInPlane{ { 0, 0, 2 * halfExtent } };
    constexpr std::array<TypeParam, 3> planeNormal{ { 0, 0, 1 } };

    EXPECT_EQ(-1, dense_mapping::Geometry::details::allCornersInSameHalfSpace(planeNormal, pointInPlane, boxCenter, halfExtent));
}

TYPED_TEST(FrustumIntersectsAxisAlignedBoundingBoxTest, detailsAllFrustumCornersInSameHalfSpacePositiveSide) {
    constexpr TypeParam alpha = 15 * M_PI / 180;
    constexpr std::array<TypeParam, 3> zero{ { 0, 0, 0 } };
    const auto frustum = dense_mapping::Geometry::frustum(zero, zero, alpha, alpha);

    constexpr std::array<TypeParam, 3> normal{ { 0, 0, 1 } };
    constexpr std::array<TypeParam, 3> point{ { 0, 0, -1 } };
    EXPECT_EQ(1, dense_mapping::Geometry::details::allCornersInSameHalfSpace(normal, point, frustum));
}

TYPED_TEST(FrustumIntersectsAxisAlignedBoundingBoxTest, detailsAllFrustumCornersInSameHalfSpacePositiveSideTouching) {
    constexpr TypeParam alpha = 15 * M_PI / 180;
    constexpr std::array<TypeParam, 3> zero{ { 0, 0, 0 } };
    const auto frustum = dense_mapping::Geometry::frustum(zero, zero, alpha, alpha);

    constexpr std::array<TypeParam, 3> normal{ { 0, 0, 1 } };
    constexpr std::array<TypeParam, 3> point{ { 0, 0, 0 } };
    EXPECT_EQ(1, dense_mapping::Geometry::details::allCornersInSameHalfSpace(normal, point, frustum));
}

TYPED_TEST(FrustumIntersectsAxisAlignedBoundingBoxTest, detailsAllFrustumCornersInSameHalfStraddling) {
    constexpr TypeParam alpha = 15 * M_PI / 180;
    constexpr std::array<TypeParam, 3> zero{ { 0, 0, 0 } };
    const auto frustum = dense_mapping::Geometry::frustum(zero, zero, alpha, alpha);

    constexpr std::array<TypeParam, 3> normal{ { 0, 0, 1 } };
    constexpr std::array<TypeParam, 3> point{ { 0, 0, 1 } };
    EXPECT_EQ(0, dense_mapping::Geometry::details::allCornersInSameHalfSpace(normal, point, frustum));
}

TYPED_TEST(FrustumIntersectsAxisAlignedBoundingBoxTest, detailsAllFrustumCornersInSameHalfSpaceNegativeSideTouching) {
    constexpr TypeParam alpha = 15 * M_PI / 180;
    constexpr std::array<TypeParam, 3> zero{ { 0, 0, 0 } };
    const auto frustum = dense_mapping::Geometry::frustum(zero, zero, alpha, alpha);

    constexpr std::array<TypeParam, 3> normal{ { 0, 0, -1 } };
    constexpr std::array<TypeParam, 3> point{ { 0, 0, 0 } };
    EXPECT_EQ(-1, dense_mapping::Geometry::details::allCornersInSameHalfSpace(normal, point, frustum));
}

TYPED_TEST(FrustumIntersectsAxisAlignedBoundingBoxTest, detailsAllFrustumCornersInSameHalfSpaceNegativeSide) {
    constexpr TypeParam alpha = 15 * M_PI / 180;
    constexpr std::array<TypeParam, 3> zero{ { 0, 0, 0 } };
    const auto frustum = dense_mapping::Geometry::frustum(zero, zero, alpha, alpha);

    constexpr std::array<TypeParam, 3> normal{ { 0, 0, -1 } };
    constexpr std::array<TypeParam, 3> point{ { 0, 0, -1 } };
    EXPECT_EQ(-1, dense_mapping::Geometry::details::allCornersInSameHalfSpace(normal, point, frustum));
}

TYPED_TEST(FrustumIntersectsAxisAlignedBoundingBoxTest, happyPathBoxWhollyContainedInFrustum) {
    constexpr TypeParam z = 10;
    constexpr std::array<TypeParam, 3> frustumCenter{ { 0, 0, 0 } };
    constexpr std::array<TypeParam, 3> zero{ { 0, 0, 0 } };

    for (int alphaDeg = 1; alphaDeg < 90; ++alphaDeg) {
        // Horizontal half FOV
        const TypeParam alpha = alphaDeg * M_PI / 180;

        for (int betaDeg = 1; betaDeg < 90; ++betaDeg) {
            const TypeParam beta = betaDeg * M_PI / 180;

            const TypeParam halfSpan = z * std::tan(std::min(alpha, beta));
            const std::array<TypeParam, 3> boxCenter{ { 0, 0, z + 2 * halfSpan } };
            const auto frustum = dense_mapping::Geometry::frustum(frustumCenter, zero, alpha, beta);
            EXPECT_TRUE(dense_mapping::Geometry::frustumIntersectsAxisAlignedBoundingBox(frustum, boxCenter, halfSpan))
                << " @ alpha = " << alphaDeg << ", beta = " << betaDeg;
        }
    }
}

TYPED_TEST(FrustumIntersectsAxisAlignedBoundingBoxTest, edgeCaseBoxTouchesFrustum) {
    constexpr TypeParam z = 10;
    constexpr std::array<TypeParam, 3> frustumCenter{ { 0, 0, 0 } };
    constexpr std::array<TypeParam, 3> zero{ { 0, 0, 0 } };

    for (int alphaDeg = 1; alphaDeg < 90; ++alphaDeg) {
        // Horizontal half FOV
        const TypeParam alpha = alphaDeg * M_PI / 180;

        for (int betaDeg = 1; betaDeg < 90; ++betaDeg) {
            const TypeParam beta = betaDeg * M_PI / 180;

            const TypeParam halfSpan = z * std::tan(std::min(alpha, beta));
            const std::array<TypeParam, 3> boxCenter{ { 0, 0, z + halfSpan } };
            const auto frustum = dense_mapping::Geometry::frustum(frustumCenter, zero, alpha, beta);
            EXPECT_TRUE(dense_mapping::Geometry::frustumIntersectsAxisAlignedBoundingBox(frustum, boxCenter, halfSpan))
                << " @ alpha = " << alphaDeg << ", beta = " << betaDeg;
        }
    }
}

TYPED_TEST(FrustumIntersectsAxisAlignedBoundingBoxTest, happyPathBoxPartiallyContainedInFrustum) {
    constexpr TypeParam z = 10;
    constexpr std::array<TypeParam, 3> frustumCenter{ { 0, 0, 0 } };
    constexpr std::array<TypeParam, 3> zero{ { 0, 0, 0 } };

    for (int alphaDeg = 1; alphaDeg < 90; ++alphaDeg) {
        // Horizontal half FOV
        const TypeParam alpha = alphaDeg * M_PI / 180;

        for (int betaDeg = 1; betaDeg < 90; ++betaDeg) {
            const TypeParam beta = betaDeg * M_PI / 180;

            const TypeParam halfSpan = z * std::tan(std::max(alpha, beta));
            const std::array<TypeParam, 3> boxCenter{ { 0, 0, z + halfSpan } };
            const auto frustum = dense_mapping::Geometry::frustum(frustumCenter, zero, alpha, beta);

            EXPECT_TRUE(dense_mapping::Geometry::frustumIntersectsAxisAlignedBoundingBox(frustum, boxCenter, halfSpan))
                << " @ alpha = " << alphaDeg << ", beta = " << betaDeg;
        }
    }
}

TYPED_TEST(FrustumIntersectsAxisAlignedBoundingBoxTest, happyPathBoxWhollyOutsideFrustum) {
    constexpr TypeParam z = 10;
    constexpr std::array<TypeParam, 3> frustumCenter{ { 0, 0, 0 } };
    constexpr std::array<TypeParam, 3> zero{ { 0, 0, 0 } };

    for (int alphaDeg = 1; alphaDeg < 90; ++alphaDeg) {
        // Horizontal half FOV
        const TypeParam alpha = alphaDeg * M_PI / 180;

        for (int betaDeg = 1; betaDeg < 90; ++betaDeg) {
            const TypeParam beta = betaDeg * M_PI / 180;

            const TypeParam halfSpan = z * std::tan(std::min(alpha, beta));

            const std::array<TypeParam, 3> boxCenter{ { 0, 0, -z - halfSpan - 2 } }; // further back so that there is separation
            const auto frustum = dense_mapping::Geometry::frustum(frustumCenter, zero, alpha, beta);
            EXPECT_FALSE(dense_mapping::Geometry::frustumIntersectsAxisAlignedBoundingBox(frustum, boxCenter, halfSpan))
                << " @ alpha = " << alphaDeg << ", beta = " << betaDeg;
        }
    }
}

TYPED_TEST(FrustumIntersectsAxisAlignedBoundingBoxTest, edgeCaseDistjointBoxCornersTouchFrustumPlanes) {
    constexpr TypeParam z = 10;
    constexpr std::array<TypeParam, 3> frustumCenter{ { 0, 0, 0 } };
    constexpr std::array<TypeParam, 3> zero{ { 0, 0, 0 } };

    for (int alphaDeg = 1; alphaDeg < 90; ++alphaDeg) {
        // Horizontal half FOV
        const TypeParam alpha = alphaDeg * M_PI / 180;

        for (int betaDeg = 1; betaDeg < 90; ++betaDeg) {
            const TypeParam beta = betaDeg * M_PI / 180;

            // Using this construction, we touch at least two of the frustum planes in the -z side
            const TypeParam halfSpan = z * std::tan(std::min(alpha, beta));
            const std::array<TypeParam, 3> boxCenter{ { 0, 0, -z - halfSpan } };
            const auto frustum = dense_mapping::Geometry::frustum(frustumCenter, zero, alpha, beta);

            EXPECT_FALSE(dense_mapping::Geometry::frustumIntersectsAxisAlignedBoundingBox(frustum, boxCenter, halfSpan))
                << " @ alpha = " << alphaDeg << ", beta = " << betaDeg;
        }
    }
}
}