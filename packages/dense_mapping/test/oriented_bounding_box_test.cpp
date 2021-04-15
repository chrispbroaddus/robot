#include "../include/box_geometry.h"

#include <memory>

#include "gtest/gtest.h"

namespace {
template <typename T> class OrientedBoundingBoxTest : public ::testing::Test {};

using TypesToTest = ::testing::Types<float, double>;

TYPED_TEST_CASE(OrientedBoundingBoxTest, TypesToTest);

TYPED_TEST(OrientedBoundingBoxTest, constructorHappyPath) {
    using oriented_box_type = dense_mapping::Geometry::OrientedBoundingBox<TypeParam>;

    constexpr std::array<TypeParam, 3> center{ { 1, 2, 3 } };
    constexpr std::array<TypeParam, 3> xDirection{ { 1, 0, 0 } };
    constexpr std::array<TypeParam, 3> yDirection{ { 0, 1, 0 } };
    constexpr std::array<TypeParam, 3> zDirection{ { 0, 0, 1 } };
    constexpr TypeParam xExtent = 4;
    constexpr TypeParam yExtent = 5;
    constexpr TypeParam zExtent = 6;

    oriented_box_type box(center, xDirection, yDirection, xExtent, yExtent, zExtent);

    EXPECT_EQ(center[0], box.m_center[0]);
    EXPECT_EQ(center[1], box.m_center[1]);
    EXPECT_EQ(center[2], box.m_center[2]);

    EXPECT_EQ(xDirection[0], box.m_xDirection[0]);
    EXPECT_EQ(xDirection[1], box.m_xDirection[1]);
    EXPECT_EQ(xDirection[2], box.m_xDirection[2]);

    EXPECT_EQ(yDirection[0], box.m_yDirection[0]);
    EXPECT_EQ(yDirection[1], box.m_yDirection[1]);
    EXPECT_EQ(yDirection[2], box.m_yDirection[2]);

    EXPECT_EQ(zDirection[0], box.m_zDirection[0]);
    EXPECT_EQ(zDirection[1], box.m_zDirection[1]);
    EXPECT_EQ(zDirection[2], box.m_zDirection[2]);

    EXPECT_EQ(xExtent, box.m_xExtent);
    EXPECT_EQ(yExtent, box.m_yExtent);
    EXPECT_EQ(zExtent, box.m_zExtent);
}

TYPED_TEST(OrientedBoundingBoxTest, constructorThrowsOnInvalidCenter) {
    using oriented_box_type = dense_mapping::Geometry::OrientedBoundingBox<TypeParam>;

    constexpr TypeParam invalidValues[] = { std::numeric_limits<TypeParam>::infinity(), -std::numeric_limits<TypeParam>::infinity(),
        std::numeric_limits<TypeParam>::quiet_NaN() };

    constexpr std::array<TypeParam, 3> xDirection{ { 1, 0, 0 } };
    constexpr std::array<TypeParam, 3> yDirection{ { 0, 1, 0 } };
    constexpr TypeParam xExtent = 4;
    constexpr TypeParam yExtent = 5;
    constexpr TypeParam zExtent = 6;

    for (const auto value : invalidValues) {
        for (size_t index = 0; index < 3; ++index) {
            std::array<TypeParam, 3> center{ { 1, 2, 3 } };
            center[index] = value;

            EXPECT_THROW(
                std::make_shared<oriented_box_type>(center, xDirection, yDirection, xExtent, yExtent, zExtent), std::runtime_error);
        }
    }
}

TYPED_TEST(OrientedBoundingBoxTest, constructorThrowsOnInvalidXDirection) {
    using oriented_box_type = dense_mapping::Geometry::OrientedBoundingBox<TypeParam>;

    constexpr TypeParam invalidValues[] = { std::numeric_limits<TypeParam>::infinity(), -std::numeric_limits<TypeParam>::infinity(),
        std::numeric_limits<TypeParam>::quiet_NaN(), std::numeric_limits<TypeParam>::signaling_NaN() };

    constexpr std::array<TypeParam, 3> center{ { 1, 2, 3 } };
    constexpr std::array<TypeParam, 3> yDirection{ { 0, 1, 0 } };
    constexpr TypeParam xExtent = 4;
    constexpr TypeParam yExtent = 5;
    constexpr TypeParam zExtent = 6;

    for (const auto value : invalidValues) {
        std::array<TypeParam, 3> xDirection{ { 1, 0, 0 } };
        for (size_t index = 0; index < 3; ++index) {
            xDirection[index] = value;

            EXPECT_THROW(
                std::make_shared<oriented_box_type>(center, xDirection, yDirection, xExtent, yExtent, zExtent), std::runtime_error);
        }
    }
}

TYPED_TEST(OrientedBoundingBoxTest, constructorThrowsOnInvalidYDirection) {
    using oriented_box_type = dense_mapping::Geometry::OrientedBoundingBox<TypeParam>;

    constexpr std::array<TypeParam, 3> center{ { 1, 2, 3 } };
    constexpr std::array<TypeParam, 3> xDirection{ { 1, 0, 0 } };
    constexpr TypeParam xExtent = 4;
    constexpr TypeParam yExtent = 5;
    constexpr TypeParam zExtent = 6;

    constexpr TypeParam invalidValues[] = { std::numeric_limits<TypeParam>::infinity(), -std::numeric_limits<TypeParam>::infinity(),
        std::numeric_limits<TypeParam>::quiet_NaN(), std::numeric_limits<TypeParam>::signaling_NaN() };

    for (const auto value : invalidValues) {
        for (size_t dimension = 0; dimension < 3; ++dimension) {
            std::array<TypeParam, 3> yDirection{ { 0, 1, 0 } };

            yDirection[dimension] = value;

            EXPECT_THROW(
                std::make_shared<oriented_box_type>(center, xDirection, yDirection, xExtent, yExtent, zExtent), std::runtime_error);
        }
    }
}

TYPED_TEST(OrientedBoundingBoxTest, constructorThrowsOnZeroXDirection) {
    using oriented_box_type = dense_mapping::Geometry::OrientedBoundingBox<TypeParam>;

    constexpr std::array<TypeParam, 3> center{ { 1, 2, 3 } };
    constexpr std::array<TypeParam, 3> xDirection{ { std::numeric_limits<TypeParam>::denorm_min(), 0, 0 } };
    constexpr std::array<TypeParam, 3> yDirection{ { 0, 1, 0 } };
    constexpr TypeParam xExtent = 4;
    constexpr TypeParam yExtent = 5;
    constexpr TypeParam zExtent = 6;

    EXPECT_THROW(std::make_shared<oriented_box_type>(center, xDirection, yDirection, xExtent, yExtent, zExtent), std::runtime_error);
}

TYPED_TEST(OrientedBoundingBoxTest, constructorThrowsOnZeroYDirection) {
    using oriented_box_type = dense_mapping::Geometry::OrientedBoundingBox<TypeParam>;

    constexpr std::array<TypeParam, 3> center{ { 1, 2, 3 } };
    constexpr std::array<TypeParam, 3> xDirection{ { 1, 0, 0 } };
    constexpr std::array<TypeParam, 3> yDirection{ { 0, std::numeric_limits<TypeParam>::denorm_min(), 0 } };
    constexpr TypeParam xExtent = 4;
    constexpr TypeParam yExtent = 5;
    constexpr TypeParam zExtent = 6;

    EXPECT_THROW(std::make_shared<oriented_box_type>(center, xDirection, yDirection, xExtent, yExtent, zExtent), std::runtime_error);
}

TYPED_TEST(OrientedBoundingBoxTest, constructorThrowsOnXNotPerpendicularToY) {
    using oriented_box_type = dense_mapping::Geometry::OrientedBoundingBox<TypeParam>;

    constexpr std::array<TypeParam, 3> center{ { 1, 2, 3 } };
    constexpr std::array<TypeParam, 3> xDirection{ { 1, 0, 0 } };
    constexpr TypeParam xExtent = 4;
    constexpr TypeParam yExtent = 5;
    constexpr TypeParam zExtent = 6;

    EXPECT_THROW(std::make_shared<oriented_box_type>(center, xDirection, xDirection, xExtent, yExtent, zExtent), std::runtime_error);
}

TYPED_TEST(OrientedBoundingBoxTest, constructorThrowsOnInvalidXExtent) {
    using oriented_box_type = dense_mapping::Geometry::OrientedBoundingBox<TypeParam>;

    constexpr std::array<TypeParam, 3> center{ { 1, 2, 3 } };
    constexpr std::array<TypeParam, 3> xDirection{ { 1, 0, 0 } };
    constexpr std::array<TypeParam, 3> yDirection{ { 0, 1, 0 } };
    constexpr TypeParam yExtent = 5;
    constexpr TypeParam zExtent = 6;

    constexpr TypeParam invalidValues[] = { std::numeric_limits<TypeParam>::infinity(), -std::numeric_limits<TypeParam>::infinity(),
        std::numeric_limits<TypeParam>::quiet_NaN(), std::numeric_limits<TypeParam>::signaling_NaN(), -1, 0,
        std::numeric_limits<TypeParam>::denorm_min(), std::numeric_limits<TypeParam>::epsilon() };

    for (const auto xExtent : invalidValues) {
        EXPECT_THROW(std::make_shared<oriented_box_type>(center, xDirection, yDirection, xExtent, yExtent, zExtent), std::runtime_error);
    }
}

TYPED_TEST(OrientedBoundingBoxTest, constructorThrowsOnInvalidYExtent) {
    using oriented_box_type = dense_mapping::Geometry::OrientedBoundingBox<TypeParam>;

    constexpr std::array<TypeParam, 3> center{ { 1, 2, 3 } };
    constexpr std::array<TypeParam, 3> xDirection{ { 1, 0, 0 } };
    constexpr std::array<TypeParam, 3> yDirection{ { 0, 1, 0 } };
    constexpr TypeParam xExtent = 5;
    constexpr TypeParam zExtent = 6;

    constexpr TypeParam invalidValues[] = { std::numeric_limits<TypeParam>::infinity(), -std::numeric_limits<TypeParam>::infinity(),
        std::numeric_limits<TypeParam>::quiet_NaN(), std::numeric_limits<TypeParam>::signaling_NaN(), -1, 0,
        std::numeric_limits<TypeParam>::denorm_min(), std::numeric_limits<TypeParam>::epsilon() };

    for (const auto yExtent : invalidValues) {
        EXPECT_THROW(std::make_shared<oriented_box_type>(center, xDirection, yDirection, xExtent, yExtent, zExtent), std::runtime_error);
    }
}

TYPED_TEST(OrientedBoundingBoxTest, constructorThrowsOnInvalidZExtent) {
    using oriented_box_type = dense_mapping::Geometry::OrientedBoundingBox<TypeParam>;

    constexpr std::array<TypeParam, 3> center{ { 1, 2, 3 } };
    constexpr std::array<TypeParam, 3> xDirection{ { 1, 0, 0 } };
    constexpr std::array<TypeParam, 3> yDirection{ { 0, 1, 0 } };
    constexpr TypeParam xExtent = 5;
    constexpr TypeParam yExtent = 6;

    constexpr TypeParam invalidValues[] = { std::numeric_limits<TypeParam>::infinity(), -std::numeric_limits<TypeParam>::infinity(),
        std::numeric_limits<TypeParam>::quiet_NaN(), std::numeric_limits<TypeParam>::signaling_NaN(), -1, 0,
        std::numeric_limits<TypeParam>::denorm_min(), std::numeric_limits<TypeParam>::epsilon() };

    for (const auto zExtent : invalidValues) {
        EXPECT_THROW(std::make_shared<oriented_box_type>(center, xDirection, yDirection, xExtent, yExtent, zExtent), std::runtime_error);
    }
}
}