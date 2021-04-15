#include "../include/box_geometry.h"
#include "gtest/gtest.h"
#include <cmath>
#include <random>

namespace {
template <typename T> class LinearAlgebraTest : public ::testing::Test {};

using TypesToTest = ::testing::Types<float, double>;
TYPED_TEST_CASE(LinearAlgebraTest, TypesToTest);

TYPED_TEST(LinearAlgebraTest, crossProductOfParalellVectorsIsZero) {
    constexpr auto epsilon = std::numeric_limits<TypeParam>::epsilon();

    for (int thetaDeg = -180; thetaDeg < 180; ++thetaDeg) {
        const TypeParam theta = thetaDeg * M_PI / 180;
        const auto sinT = std::sin(theta);
        const auto cosT = std::cos(theta);

        for (int phiDeg = -90; phiDeg < 90; ++phiDeg) {
            const TypeParam phi = phiDeg * M_PI / 180;
            const auto sinP = std::sin(phi);
            const auto cosP = std::cos(phi);

            const std::array<TypeParam, 3> a{ { sinT * cosP, sinT * sinP, cosT } };
            const std::array<TypeParam, 3> b{ { -a[0], -a[1], -a[2] } };

            const auto axa = dense_mapping::Geometry::crossProduct(a, a);
            EXPECT_NEAR(0, dense_mapping::Geometry::normL2(axa), epsilon);

            const auto axb = dense_mapping::Geometry::crossProduct(a, b);
            EXPECT_NEAR(0, dense_mapping::Geometry::normL2(axb), epsilon);
        }
    }
}

TYPED_TEST(LinearAlgebraTest, crossProductIsPerpendicularToBasisVectors) {
    constexpr auto epsilon = std::numeric_limits<TypeParam>::epsilon();
    constexpr size_t numSamples = 10000;
    constexpr TypeParam pi = M_PI;

    std::mt19937 rng(0);
    std::uniform_real_distribution<TypeParam> theta(-pi, pi);
    std::uniform_real_distribution<TypeParam> phi(-pi / 2, pi / 2);

    for (size_t i = 0; i < numSamples; ++i) {
        const auto thetaA = theta(rng);
        const auto phiA = theta(rng);
        const auto sinTA = std::sin(thetaA);
        const auto cosTA = std::cos(thetaA);
        const auto sinPA = std::sin(phiA);
        const auto cosPA = std::cos(phiA);
        const std::array<TypeParam, 3> a{ { sinTA * cosPA, sinTA * sinPA, cosTA } };

        const auto thetaB = theta(rng);
        const auto phiB = theta(rng);
        const auto sinTB = std::sin(thetaB);
        const auto cosTB = std::cos(thetaB);
        const auto sinPB = std::sin(phiB);
        const auto cosPB = std::cos(phiB);
        const std::array<TypeParam, 3> b{ { sinTB * cosPB, sinTB * sinPB, cosTB } };

        const auto c1 = dense_mapping::Geometry::crossProduct(a, b);
        const auto c2 = dense_mapping::Geometry::crossProduct(b, a);

        // Verify that c1 = -c2
        EXPECT_NEAR(-c1[0], c2[0], epsilon);
        EXPECT_NEAR(-c1[1], c2[1], epsilon);
        EXPECT_NEAR(-c1[2], c2[2], epsilon);

        EXPECT_NEAR(0, dense_mapping::Geometry::innerProduct(a, c1), epsilon);
        EXPECT_NEAR(0, dense_mapping::Geometry::innerProduct(b, c1), epsilon);
        EXPECT_NEAR(0, dense_mapping::Geometry::innerProduct(a, c2), epsilon);
        EXPECT_NEAR(0, dense_mapping::Geometry::innerProduct(b, c2), epsilon);
    }
}
}