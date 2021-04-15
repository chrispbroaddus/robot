#include "../include/box_geometry.h"

#include <gtest/gtest.h>

namespace {

template <typename T> class RotationMatrixFromRotationVectorTest : public ::testing::Test {};

using TypesToTest = ::testing::Types<double, float>;

TYPED_TEST_CASE(RotationMatrixFromRotationVectorTest, TypesToTest);

TYPED_TEST(RotationMatrixFromRotationVectorTest, checkZeroVectorYieldsIdentity) {
    constexpr std::array<TypeParam, 3> zero{ { 0, 0, 0 } };
    const auto matrix = dense_mapping::Geometry::rotationMatrixFromRotationVector(zero);

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            EXPECT_EQ((i == j ? 1 : 0), matrix[3 * i + j]);
        }
    }
}

TYPED_TEST(RotationMatrixFromRotationVectorTest, checkDeterminant) {
    constexpr TypeParam eps = 1e-6;

    for (int thetaDeg = -180; thetaDeg <= 180; thetaDeg += 5) {
        const TypeParam theta = thetaDeg * M_PI / 180;
        const TypeParam sinT = std::sin(theta);
        const TypeParam cosT = std::cos(theta);

        for (int phiDeg = -90; phiDeg <= 90; phiDeg += 5) {
            const TypeParam phi = phiDeg * M_PI / 180;
            const TypeParam sinP = std::sin(phi);
            const TypeParam cosP = std::cos(phi);

            const std::array<TypeParam, 3> axis{ { sinT * cosP, sinT * sinP, cosT } };
            const TypeParam axisNorm = dense_mapping::Geometry::normL2(axis);

            for (int alphaDeg = 0; alphaDeg < 180; alphaDeg += 5) {
                const TypeParam alpha = alphaDeg * M_PI / 180;
                const TypeParam scale = alpha / axisNorm;
                const std::array<TypeParam, 3> rotationVector{ { axis[0] * scale, axis[1] * scale, axis[2] * scale } };
                const std::array<TypeParam, 3 * 3> rotationMatrix
                    = dense_mapping::Geometry::rotationMatrixFromRotationVector(rotationVector);

                const TypeParam det = rotationMatrix[0] * (rotationMatrix[4] * rotationMatrix[8] - rotationMatrix[5] * rotationMatrix[7])
                    - rotationMatrix[1] * (rotationMatrix[3] * rotationMatrix[8] - rotationMatrix[5] * rotationMatrix[6])
                    + rotationMatrix[2] * (rotationMatrix[3] * rotationMatrix[7] - rotationMatrix[4] * rotationMatrix[6]);
                EXPECT_NEAR(1, det, eps);
            }
        }
    }
}

TYPED_TEST(RotationMatrixFromRotationVectorTest, columnsHaveUnitNorm) {
    constexpr TypeParam eps = 1e-6;

    for (int thetaDeg = -180; thetaDeg <= 180; thetaDeg += 5) {
        const TypeParam theta = thetaDeg * M_PI / 180;
        const TypeParam sinT = std::sin(theta);
        const TypeParam cosT = std::cos(theta);

        for (int phiDeg = -90; phiDeg <= 90; phiDeg += 5) {
            const TypeParam phi = phiDeg * M_PI / 180;
            const TypeParam sinP = std::sin(phi);
            const TypeParam cosP = std::cos(phi);

            const std::array<TypeParam, 3> axis{ { sinT * cosP, sinT * sinP, cosT } };
            const TypeParam axisNorm = dense_mapping::Geometry::normL2(axis);

            for (int alphaDeg = 0; alphaDeg < 180; alphaDeg += 5) {
                const TypeParam alpha = alphaDeg * M_PI / 180;
                const TypeParam scale = alpha / axisNorm;
                const std::array<TypeParam, 3> rotationVector{ { axis[0] * scale, axis[1] * scale, axis[2] * scale } };
                const std::array<TypeParam, 3 * 3> rotationMatrix
                    = dense_mapping::Geometry::rotationMatrixFromRotationVector(rotationVector);

                const auto c1 = rotationMatrix[0 * 3 + 0] * rotationMatrix[0 * 3 + 0]
                    + rotationMatrix[1 * 3 + 0] * rotationMatrix[1 * 3 + 0] + rotationMatrix[2 * 3 + 0] * rotationMatrix[2 * 3 + 0];
                const auto c2 = rotationMatrix[0 * 3 + 1] * rotationMatrix[0 * 3 + 1]
                    + rotationMatrix[1 * 3 + 1] * rotationMatrix[1 * 3 + 1] + rotationMatrix[2 * 3 + 1] * rotationMatrix[2 * 3 + 1];
                const auto c3 = rotationMatrix[0 * 3 + 2] * rotationMatrix[0 * 3 + 2]
                    + rotationMatrix[1 * 3 + 2] * rotationMatrix[1 * 3 + 2] + rotationMatrix[2 * 3 + 2] * rotationMatrix[2 * 3 + 2];

                EXPECT_NEAR(1, c1, eps);
                EXPECT_NEAR(1, c2, eps);
                EXPECT_NEAR(1, c3, eps);
            }
        }
    }
}

TYPED_TEST(RotationMatrixFromRotationVectorTest, rowsHaveUnitNorm) {
    constexpr TypeParam eps = 1e-6;

    for (int thetaDeg = -180; thetaDeg <= 180; thetaDeg += 5) {
        const TypeParam theta = thetaDeg * M_PI / 180;
        const TypeParam sinT = std::sin(theta);
        const TypeParam cosT = std::cos(theta);

        for (int phiDeg = -90; phiDeg <= 90; phiDeg += 5) {
            const TypeParam phi = phiDeg * M_PI / 180;
            const TypeParam sinP = std::sin(phi);
            const TypeParam cosP = std::cos(phi);

            const std::array<TypeParam, 3> axis{ { sinT * cosP, sinT * sinP, cosT } };
            const TypeParam axisNorm = dense_mapping::Geometry::normL2(axis);

            for (int alphaDeg = 0; alphaDeg < 180; alphaDeg += 5) {
                const TypeParam alpha = alphaDeg * M_PI / 180;
                const TypeParam scale = alpha / axisNorm;
                const std::array<TypeParam, 3> rotationVector{ { axis[0] * scale, axis[1] * scale, axis[2] * scale } };
                const std::array<TypeParam, 3 * 3> rotationMatrix
                    = dense_mapping::Geometry::rotationMatrixFromRotationVector(rotationVector);

                const auto r1 = rotationMatrix[0 * 3 + 0] * rotationMatrix[0 * 3 + 0]
                    + rotationMatrix[0 * 3 + 1] * rotationMatrix[0 * 3 + 1] + rotationMatrix[0 * 3 + 2] * rotationMatrix[0 * 3 + 2];
                const auto r2 = rotationMatrix[1 * 3 + 0] * rotationMatrix[1 * 3 + 0]
                    + rotationMatrix[1 * 3 + 1] * rotationMatrix[1 * 3 + 1] + rotationMatrix[1 * 3 + 2] * rotationMatrix[1 * 3 + 2];
                const auto r3 = rotationMatrix[2 * 3 + 0] * rotationMatrix[2 * 3 + 0]
                    + rotationMatrix[2 * 3 + 1] * rotationMatrix[2 * 3 + 1] + rotationMatrix[2 * 3 + 2] * rotationMatrix[2 * 3 + 2];

                EXPECT_NEAR(1, r1, eps);
                EXPECT_NEAR(1, r2, eps);
                EXPECT_NEAR(1, r3, eps);
            }
        }
    }
}

TYPED_TEST(RotationMatrixFromRotationVectorTest, checkOrthonormality) {
    constexpr TypeParam eps = 1e-6;

    for (int thetaDeg = -180; thetaDeg <= 180; thetaDeg += 5) {
        const TypeParam theta = thetaDeg * M_PI / 180;
        const TypeParam sinT = std::sin(theta);
        const TypeParam cosT = std::cos(theta);

        for (int phiDeg = -90; phiDeg <= 90; phiDeg += 5) {
            const TypeParam phi = phiDeg * M_PI / 180;
            const TypeParam sinP = std::sin(phi);
            const TypeParam cosP = std::cos(phi);

            const std::array<TypeParam, 3> axis{ { sinT * cosP, sinT * sinP, cosT } };
            const TypeParam axisNorm = dense_mapping::Geometry::normL2(axis);

            for (int alphaDeg = 0; alphaDeg < 180; alphaDeg += 5) {
                const TypeParam alpha = alphaDeg * M_PI / 180;
                const TypeParam scale = alpha / axisNorm;
                const std::array<TypeParam, 3> rotationVector{ { axis[0] * scale, axis[1] * scale, axis[2] * scale } };
                const std::array<TypeParam, 3 * 3> rotationMatrix
                    = dense_mapping::Geometry::rotationMatrixFromRotationVector(rotationVector);

                // Compute X = RR'
                for (int i = 0; i < 3; ++i) {
                    for (int j = 0; j < 3; ++j) {
                        TypeParam t = 0;

                        for (int k = 0; k < 3; ++k) {
                            t += rotationMatrix[i * 3 + k] * rotationMatrix[j * 3 + k];
                        }

                        EXPECT_NEAR(i == j ? 1 : 0, t, eps) << "i = " << i << ", j = " << j;
                    }
                }
            }
        }
    }
}
}