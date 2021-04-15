#include "../include/heightmap.h"

#include "gtest/gtest.h"

#include <random>

namespace {
template <typename T> class HeightMapTest : public ::testing::Test {};

using TypesToTest = ::testing::Types<float, double>;

TYPED_TEST_CASE(HeightMapTest, TypesToTest);

TYPED_TEST(HeightMapTest, details_containedInNorthWestChild) {
    constexpr TypeParam centerX = 2;
    constexpr TypeParam centerY = 5;
    constexpr TypeParam extentX = 10;
    constexpr TypeParam extentY = 10;
    constexpr TypeParam epsilon = std::numeric_limits<TypeParam>::epsilon() * 1000;
    constexpr TypeParam signX = -1;
    constexpr TypeParam signY = -1;

    EXPECT_TRUE(dense_mapping::heightmap::details::containedInNorthWestChild(
        centerX + signX * (extentX - epsilon), centerY + signY * (extentY - epsilon), centerX, centerY, extentX, extentY));
    EXPECT_TRUE(dense_mapping::heightmap::details::containedInNorthWestChild(
        centerX + signX * epsilon, centerY + signY * (extentY - epsilon), centerX, centerY, extentX, extentY));
    EXPECT_TRUE(dense_mapping::heightmap::details::containedInNorthWestChild(
        centerX + signX * (extentX - epsilon), centerY + signY * epsilon, centerX, centerY, extentX, extentY));
    EXPECT_TRUE(dense_mapping::heightmap::details::containedInNorthWestChild(
        centerX + signX * epsilon, centerY + signY * epsilon, centerX, centerY, extentX, extentY));

    // Correct quadrant but outside extents
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInNorthWestChild(
        centerX + signX * (extentX + epsilon), centerY + signY * epsilon, centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInNorthWestChild(
        centerX + signX * epsilon, centerY + signY * (extentY + epsilon), centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInNorthWestChild(
        centerX + signX * (extentX + epsilon), centerY + signY * (extentY + epsilon), centerX, centerY, extentX, extentY));

    // Wrong quadrants
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInNorthWestChild(
        centerX + signX * extentX / 2, centerY - signY * extentY / 2, centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInNorthWestChild(
        centerX - signX * extentX / 2, centerY + signY * extentY / 2, centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInNorthWestChild(
        centerX - signX * extentX / 2, centerY - signY * extentY / 2, centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInNorthWestChild(
        centerX + signX * epsilon, centerY - signY * epsilon, centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInNorthWestChild(
        centerX - signX * epsilon, centerY + signY * epsilon, centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInNorthWestChild(
        centerX - signX * epsilon, centerY - signY * epsilon, centerX, centerY, extentX, extentY));
}

TYPED_TEST(HeightMapTest, details_containedInSouthWestChild) {
    constexpr TypeParam centerX = 2;
    constexpr TypeParam centerY = 5;
    constexpr TypeParam extentX = 10;
    constexpr TypeParam extentY = 10;
    constexpr TypeParam epsilon = std::numeric_limits<TypeParam>::epsilon() * 1000;
    constexpr TypeParam signX = -1;
    constexpr TypeParam signY = 1;

    EXPECT_TRUE(dense_mapping::heightmap::details::containedInSouthWestChild(
        centerX + signX * (extentX - epsilon), centerY + signY * (extentY - epsilon), centerX, centerY, extentX, extentY));
    EXPECT_TRUE(dense_mapping::heightmap::details::containedInSouthWestChild(
        centerX + signX * epsilon, centerY + signY * (extentY - epsilon), centerX, centerY, extentX, extentY));
    EXPECT_TRUE(dense_mapping::heightmap::details::containedInSouthWestChild(
        centerX + signX * (extentX - epsilon), centerY + signY * epsilon, centerX, centerY, extentX, extentY));
    EXPECT_TRUE(dense_mapping::heightmap::details::containedInSouthWestChild(
        centerX + signX * epsilon, centerY + signY * epsilon, centerX, centerY, extentX, extentY));

    // Correct quadrant but outside extents
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInSouthWestChild(
        centerX + signX * (extentX + epsilon), centerY + signY * epsilon, centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInSouthWestChild(
        centerX + signX * epsilon, centerY + signY * (extentY + epsilon), centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInSouthWestChild(
        centerX + signX * (extentX + epsilon), centerY + signY * (extentY + epsilon), centerX, centerY, extentX, extentY));

    // Wrong quadrants
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInSouthWestChild(
        centerX + signX * extentX / 2, centerY - signY * extentY / 2, centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInSouthWestChild(
        centerX - signX * extentX / 2, centerY + signY * extentY / 2, centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInSouthWestChild(
        centerX - signX * extentX / 2, centerY - signY * extentY / 2, centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInSouthWestChild(
        centerX + signX * epsilon, centerY - signY * epsilon, centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInSouthWestChild(
        centerX - signX * epsilon, centerY + signY * epsilon, centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInSouthWestChild(
        centerX - signX * epsilon, centerY - signY * epsilon, centerX, centerY, extentX, extentY));
}

TYPED_TEST(HeightMapTest, details_containedInNorthEastChild) {
    constexpr TypeParam centerX = 2;
    constexpr TypeParam centerY = 5;
    constexpr TypeParam extentX = 10;
    constexpr TypeParam extentY = 10;
    constexpr TypeParam epsilon = std::numeric_limits<TypeParam>::epsilon() * 1000;
    constexpr TypeParam signX = 1;
    constexpr TypeParam signY = -1;

    EXPECT_TRUE(dense_mapping::heightmap::details::containedInNorthEastChild(
        centerX + signX * (extentX - epsilon), centerY + signY * (extentY - epsilon), centerX, centerY, extentX, extentY));
    EXPECT_TRUE(dense_mapping::heightmap::details::containedInNorthEastChild(
        centerX + signX * epsilon, centerY + signY * (extentY - epsilon), centerX, centerY, extentX, extentY));
    EXPECT_TRUE(dense_mapping::heightmap::details::containedInNorthEastChild(
        centerX + signX * (extentX - epsilon), centerY + signY * epsilon, centerX, centerY, extentX, extentY));
    EXPECT_TRUE(dense_mapping::heightmap::details::containedInNorthEastChild(
        centerX + signX * epsilon, centerY + signY * epsilon, centerX, centerY, extentX, extentY));

    // Correct quadrant but outside extents
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInNorthEastChild(
        centerX + signX * (extentX + epsilon), centerY + signY * epsilon, centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInNorthEastChild(
        centerX + signX * epsilon, centerY + signY * (extentY + epsilon), centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInNorthEastChild(
        centerX + signX * (extentX + epsilon), centerY + signY * (extentY + epsilon), centerX, centerY, extentX, extentY));

    // Wrong quadrants
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInNorthEastChild(
        centerX + signX * extentX / 2, centerY - signY * extentY / 2, centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInNorthEastChild(
        centerX - signX * extentX / 2, centerY + signY * extentY / 2, centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInNorthEastChild(
        centerX - signX * extentX / 2, centerY - signY * extentY / 2, centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInNorthEastChild(
        centerX + signX * epsilon, centerY - signY * epsilon, centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInNorthEastChild(
        centerX - signX * epsilon, centerY + signY * epsilon, centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInNorthEastChild(
        centerX - signX * epsilon, centerY - signY * epsilon, centerX, centerY, extentX, extentY));
}

TYPED_TEST(HeightMapTest, details_containedInSouthEastChild) {
    constexpr TypeParam centerX = 2;
    constexpr TypeParam centerY = 5;
    constexpr TypeParam extentX = 10;
    constexpr TypeParam extentY = 10;
    constexpr TypeParam epsilon = std::numeric_limits<TypeParam>::epsilon() * 1000;
    constexpr TypeParam signX = 1;
    constexpr TypeParam signY = 1;

    EXPECT_TRUE(dense_mapping::heightmap::details::containedInSouthEastChild(
        centerX + signX * (extentX - epsilon), centerY + signY * (extentY - epsilon), centerX, centerY, extentX, extentY));
    EXPECT_TRUE(dense_mapping::heightmap::details::containedInSouthEastChild(
        centerX + signX * epsilon, centerY + signY * (extentY - epsilon), centerX, centerY, extentX, extentY));
    EXPECT_TRUE(dense_mapping::heightmap::details::containedInSouthEastChild(
        centerX + signX * (extentX - epsilon), centerY + signY * epsilon, centerX, centerY, extentX, extentY));
    EXPECT_TRUE(dense_mapping::heightmap::details::containedInSouthEastChild(
        centerX + signX * epsilon, centerY + signY * epsilon, centerX, centerY, extentX, extentY));

    // Correct quadrant but outside extents
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInSouthEastChild(
        centerX + signX * (extentX + epsilon), centerY + signY * epsilon, centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInSouthEastChild(
        centerX + signX * epsilon, centerY + signY * (extentY + epsilon), centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInSouthEastChild(
        centerX + signX * (extentX + epsilon), centerY + signY * (extentY + epsilon), centerX, centerY, extentX, extentY));

    // Wrong quadrants
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInSouthEastChild(
        centerX + signX * extentX / 2, centerY - signY * extentY / 2, centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInSouthEastChild(
        centerX - signX * extentX / 2, centerY + signY * extentY / 2, centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInSouthEastChild(
        centerX - signX * extentX / 2, centerY - signY * extentY / 2, centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInSouthEastChild(
        centerX + signX * epsilon, centerY - signY * epsilon, centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInSouthEastChild(
        centerX - signX * epsilon, centerY + signY * epsilon, centerX, centerY, extentX, extentY));
    EXPECT_FALSE(dense_mapping::heightmap::details::containedInSouthEastChild(
        centerX - signX * epsilon, centerY - signY * epsilon, centerX, centerY, extentX, extentY));
}

TYPED_TEST(HeightMapTest, canInstantiate) {
    using scalar_type = TypeParam;
    using map_type = dense_mapping::heightmap::HeightMap<scalar_type>;
    map_type map(0, 0, 10, 10);
}

TYPED_TEST(HeightMapTest, throwOnInvalidXCenter) {
    using scalar_type = TypeParam;
    using map_type = dense_mapping::heightmap::HeightMap<scalar_type>;

    constexpr scalar_type invalidCenters[] = { std::numeric_limits<scalar_type>::infinity(), -std::numeric_limits<scalar_type>::infinity(),
        std::numeric_limits<scalar_type>::quiet_NaN() };

    for (auto invalidCenter : invalidCenters) {
        EXPECT_THROW(std::make_shared<map_type>(invalidCenter, 0, 1, 1), std::runtime_error) << "Center X : " << invalidCenter;
    }
}

TYPED_TEST(HeightMapTest, throwOnInvalidYCenter) {
    using scalar_type = TypeParam;
    using map_type = dense_mapping::heightmap::HeightMap<scalar_type>;

    constexpr scalar_type invalidCenters[] = { std::numeric_limits<scalar_type>::infinity(), -std::numeric_limits<scalar_type>::infinity(),
        std::numeric_limits<scalar_type>::quiet_NaN() };

    for (auto invalidCenter : invalidCenters) {
        EXPECT_THROW(std::make_shared<map_type>(0, invalidCenter, 1, 1), std::runtime_error) << "Center Y : " << invalidCenter;
    }
}

TYPED_TEST(HeightMapTest, throwOnInvalidXExtent) {
    using scalar_type = TypeParam;
    using map_type = dense_mapping::heightmap::HeightMap<scalar_type>;

    constexpr scalar_type invalidExtents[] = { std::numeric_limits<scalar_type>::infinity(), -std::numeric_limits<scalar_type>::infinity(),
        std::numeric_limits<scalar_type>::quiet_NaN(), -1, 0

    };

    for (auto invalidExtent : invalidExtents) {
        EXPECT_THROW(std::make_shared<map_type>(0, 0, invalidExtent, 1), std::runtime_error) << "Extent X : " << invalidExtent;
    }
}

TYPED_TEST(HeightMapTest, throwOnInvalidYExtent) {
    using scalar_type = TypeParam;
    using map_type = dense_mapping::heightmap::HeightMap<scalar_type>;

    constexpr scalar_type invalidExtents[] = { std::numeric_limits<scalar_type>::infinity(), -std::numeric_limits<scalar_type>::infinity(),
        std::numeric_limits<scalar_type>::quiet_NaN(), -1, 0 };

    for (auto invalidExtent : invalidExtents) {
        EXPECT_THROW(std::make_shared<map_type>(0, 0, 1, invalidExtent), std::runtime_error) << "Extent Y : " << invalidExtent;
    }
}

TYPED_TEST(HeightMapTest, queryOutsideVolumeReturnsInfinity) {
    using scalar_type = TypeParam;
    using map_type = dense_mapping::heightmap::HeightMap<scalar_type>;
    constexpr scalar_type mapCenterX = 0;
    constexpr scalar_type mapCenterY = 0;
    constexpr scalar_type mapExtentX = 10;
    constexpr scalar_type mapExtentY = 10;
    map_type map(mapCenterX, mapCenterY, mapExtentX, mapExtentY);

    map.insert(0, 0, 0);

    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(),
        map.estimateHeightAtLocation(mapCenterX + 2 * mapExtentX, mapCenterY + 2 * mapExtentY).second);
    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(), map.estimateHeightAtLocation(mapCenterX + 2 * mapExtentX, mapCenterY).second);
    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(),
        map.estimateHeightAtLocation(mapCenterX + 2 * mapExtentX, mapCenterY - 2 * mapExtentY).second);

    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(), map.estimateHeightAtLocation(mapCenterX, mapCenterY + 2 * mapExtentY).second);
    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(), map.estimateHeightAtLocation(mapCenterX, mapCenterY - 2 * mapExtentY).second);

    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(),
        map.estimateHeightAtLocation(mapCenterX - 2 * mapExtentX, mapCenterY + 2 * mapExtentY).second);
    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(), map.estimateHeightAtLocation(mapCenterX - 2 * mapExtentX, mapCenterY).second);
    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(),
        map.estimateHeightAtLocation(mapCenterX - 2 * mapExtentX, mapCenterY - 2 * mapExtentY).second);
}

TYPED_TEST(HeightMapTest, queryWithNoDataReturnsInfinity) {
    using scalar_type = TypeParam;
    using map_type = dense_mapping::heightmap::HeightMap<scalar_type>;
    constexpr scalar_type mapCenterX = 0;
    constexpr scalar_type mapCenterY = 0;
    constexpr scalar_type mapExtentX = 10;
    constexpr scalar_type mapExtentY = 10;
    map_type map(mapCenterX, mapCenterY, mapExtentX, mapExtentY);

    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(),
        map.estimateHeightAtLocation(mapCenterX + 2 * mapExtentX, mapCenterY + 2 * mapExtentY).second);
    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(), map.estimateHeightAtLocation(mapCenterX + 2 * mapExtentX, mapCenterY).second);
    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(),
        map.estimateHeightAtLocation(mapCenterX + 2 * mapExtentX, mapCenterY - 2 * mapExtentY).second);

    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(), map.estimateHeightAtLocation(mapCenterX, mapCenterY + 2 * mapExtentY).second);
    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(), map.estimateHeightAtLocation(mapCenterX, mapCenterY).second);
    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(), map.estimateHeightAtLocation(mapCenterX, mapCenterY - 2 * mapExtentY).second);

    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(),
        map.estimateHeightAtLocation(mapCenterX - 2 * mapExtentX, mapCenterY + 2 * mapExtentY).second);
    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(), map.estimateHeightAtLocation(mapCenterX - 2 * mapExtentX, mapCenterY).second);
    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(),
        map.estimateHeightAtLocation(mapCenterX - 2 * mapExtentX, mapCenterY - 2 * mapExtentY).second);
}

TYPED_TEST(HeightMapTest, queryAfterSplitInEmptyRegionReturnsInfinity) {
    using scalar_type = TypeParam;
    using map_type = dense_mapping::heightmap::HeightMap<scalar_type>;
    constexpr scalar_type mapCenterX = 0;
    constexpr scalar_type mapCenterY = 0;
    constexpr scalar_type mapExtentX = 10;
    constexpr scalar_type mapExtentY = 10;
    map_type map(mapCenterX, mapCenterY, mapExtentX, mapExtentY);

    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(),
        map.estimateHeightAtLocation(mapCenterX + 2 * mapExtentX, mapCenterY + 2 * mapExtentY).second);
    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(), map.estimateHeightAtLocation(mapCenterX + 2 * mapExtentX, mapCenterY).second);
    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(),
        map.estimateHeightAtLocation(mapCenterX + 2 * mapExtentX, mapCenterY - 2 * mapExtentY).second);

    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(), map.estimateHeightAtLocation(mapCenterX, mapCenterY + 2 * mapExtentY).second);
    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(), map.estimateHeightAtLocation(mapCenterX, mapCenterY).second);
    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(), map.estimateHeightAtLocation(mapCenterX, mapCenterY - 2 * mapExtentY).second);

    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(),
        map.estimateHeightAtLocation(mapCenterX - 2 * mapExtentX, mapCenterY + 2 * mapExtentY).second);
    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(), map.estimateHeightAtLocation(mapCenterX - 2 * mapExtentX, mapCenterY).second);
    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(),
        map.estimateHeightAtLocation(mapCenterX - 2 * mapExtentX, mapCenterY - 2 * mapExtentY).second);
}
}