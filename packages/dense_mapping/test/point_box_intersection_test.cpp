#include "packages/dense_mapping/include/box_geometry.h"
#include <gtest/gtest.h>

namespace {
template <typename T> struct PointIntersectsAxisAlignedBoundingBox {
    using scalar_type = T;
    using result_type = bool;

    static void assertTrue(result_type r) { EXPECT_TRUE(r); }

    static void assertFalse(result_type r) { EXPECT_FALSE(r); }

    static void expectEq(result_type expected, result_type actual) { EXPECT_EQ(expected, actual); }

    static constexpr result_type checkIntersection(std::array<T, 3> point, std::array<T, 3> boxCenter, T boxHalfExtent) {
        return dense_mapping::Geometry::pointIntersectsAxisAlignedBoundingBox(point, boxCenter, boxHalfExtent);
    }
};

template <typename IMPLEMENTATION_UNDER_TEST> class PointIntersectsBoxTest : public ::testing::Test {
public:
    using scalar_type = typename IMPLEMENTATION_UNDER_TEST::scalar_type;

    static constexpr std::array<scalar_type, 3> boxCenter{ { 0, 0, 0 } };
    static constexpr scalar_type boxHalfSpan = 1.0f;
    static constexpr int numStepsY = 11;
    static constexpr int numStepsZ = 11;
    static constexpr int numStepsX = 11;
};

template <typename T> constexpr std::array<typename T::scalar_type, 3> PointIntersectsBoxTest<T>::boxCenter;

template <typename T> constexpr int PointIntersectsBoxTest<T>::numStepsX;
template <typename T> constexpr int PointIntersectsBoxTest<T>::numStepsY;
template <typename T> constexpr int PointIntersectsBoxTest<T>::numStepsZ;

template <typename T> constexpr typename T::scalar_type PointIntersectsBoxTest<T>::boxHalfSpan;

using PointBoxIntersectionTypesToTest
    = ::testing::Types<PointIntersectsAxisAlignedBoundingBox<float>, PointIntersectsAxisAlignedBoundingBox<double> >;

TYPED_TEST_CASE(PointIntersectsBoxTest, PointBoxIntersectionTypesToTest);

TYPED_TEST(PointIntersectsBoxTest, checkBoxIntersections) {
    for (int xStep = -TestFixture::numStepsX; xStep <= TestFixture::numStepsX; ++xStep) {
        const auto x = 2 * xStep * TestFixture::boxHalfSpan / (TestFixture::numStepsX - 1);
        const auto inX = (x >= -TestFixture::boxHalfSpan) && (x <= TestFixture::boxHalfSpan);

        for (int yStep = -TestFixture::numStepsY; yStep <= TestFixture::numStepsY; ++yStep) {
            const auto y = 2 * yStep * TestFixture::boxHalfSpan / (TestFixture::numStepsY - 1);
            const auto inY = (y >= -TestFixture::boxHalfSpan) && (y <= TestFixture::boxHalfSpan);

            for (int zStep = -TestFixture::numStepsZ; zStep <= TestFixture::numStepsZ; ++zStep) {
                const auto z = 2 * zStep * TestFixture::boxHalfSpan / (TestFixture::numStepsZ - 1);
                const auto inZ = (z >= -TestFixture::boxHalfSpan) && (z <= TestFixture::boxHalfSpan);
                const std::array<typename TestFixture::scalar_type, 3> point{ { x, y, z } };

                TypeParam::expectEq(
                    inX && inY && inZ, TypeParam::checkIntersection(point, TestFixture::boxCenter, TestFixture::boxHalfSpan));
            }
        }
    }
}
}
