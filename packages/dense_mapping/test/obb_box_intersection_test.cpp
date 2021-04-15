#include "../include/box_geometry.h"

#include "gtest/gtest.h"

namespace {
template <typename T> class ObbBoxIntersectionTest : public ::testing::Test {};

using TypesToTest = ::testing::Types<float, double>;

TYPED_TEST_CASE(ObbBoxIntersectionTest, TypesToTest);

TYPED_TEST(ObbBoxIntersectionTest, sameCenter) {
    constexpr TypeParam extents[] = { 1, 10, 100, 10000, 10000, 100000, 1000000, 10000000 };

    constexpr size_t thetaSamples = 30;

    constexpr size_t phiSamples = 15;

    constexpr TypeParam rotationMagnitudes[] = { 0, 1 * M_PI / 180, 5 * M_PI / 180, 15 * M_PI / 180, 45 * M_PI / 180, 85 * M_PI / 180,
        135 * M_PI / 180, 165 * M_PI / 180, 175 * M_PI / 180 };

    constexpr std::array<TypeParam, 3> center{ { 0, 0, 0 } };

    // Sample the unit sphere for a rotation axis
    for (size_t thetaIdx = 0; thetaIdx < thetaSamples; ++thetaIdx) {
        const TypeParam theta = 2 * M_PI * thetaIdx / thetaSamples - M_PI;
        const auto st = std::sin(theta);
        const auto ct = std::cos(theta);

        for (size_t phiIdx = 0; phiIdx < phiSamples; ++phiIdx) {
            const TypeParam phi = M_PI * phiIdx / phiSamples - M_PI / 2;
            const auto sp = std::sin(phi);
            const auto cp = std::cos(phi);

            // Sample rotation amount
            for (const auto alpha : rotationMagnitudes) {
                const std::array<TypeParam, 3> rotation{ { alpha * st * cp, alpha * st * sp, alpha * ct } };

                // Sample extents for the obb
                for (const auto xExtent : extents) {
                    for (const auto yExtent : extents) {
                        for (const auto zExtent : extents) {
                            // Make sure that boxes that share the same center are found to intersect, regardless of the aspect ratios
                            const auto obb = dense_mapping::Geometry::orientedBoundingBox(center, rotation, xExtent, yExtent, zExtent);
                            EXPECT_TRUE(dense_mapping::Geometry::obbIntersectsAxisAlignedBoundingBox(obb, center, TypeParam(1)))
                                << "Theta = [" << theta << " / " << thetaIdx << "], phi = [" << phi << " / " << phiIdx << "], alpha = ["
                                << alpha << "], xE [" << xExtent << "], yE [" << yExtent << "], zE [" << zExtent << "]";
                        }
                    }
                }
            }
        }
    }
}

TYPED_TEST(ObbBoxIntersectionTest, allObbCornersWithinAabbNegativeHalfSpace) {
    constexpr TypeParam extents[] = { 1, 10, 100, 10000, 10000, 100000, 1000000, 10000000 };

    constexpr size_t thetaSamples = 30;

    constexpr size_t phiSamples = 15;

    constexpr TypeParam rotationMagnitudes[] = { 0, 1 * M_PI / 180, 5 * M_PI / 180, 15 * M_PI / 180, 45 * M_PI / 180, 85 * M_PI / 180,
        135 * M_PI / 180, 165 * M_PI / 180, 175 * M_PI / 180 };

    constexpr std::array<TypeParam, 3> center{ { 0, 0, 0 } };

    // Sample the unit sphere for a rotation axis
    for (size_t thetaIdx = 0; thetaIdx < thetaSamples; ++thetaIdx) {
        const TypeParam theta = 2 * M_PI * thetaIdx / thetaSamples - M_PI;
        const auto st = std::sin(theta);
        const auto ct = std::cos(theta);

        for (size_t phiIdx = 0; phiIdx < phiSamples; ++phiIdx) {
            const TypeParam phi = M_PI * phiIdx / phiSamples - M_PI / 2;
            const auto sp = std::sin(phi);
            const auto cp = std::cos(phi);

            // Sample rotation amount
            for (const auto alpha : rotationMagnitudes) {
                const std::array<TypeParam, 3> rotation{ { alpha * st * cp, alpha * st * sp, alpha * ct } };

                // Sample extents for the obb
                for (const auto xExtent : extents) {
                    for (const auto yExtent : extents) {
                        for (const auto zExtent : extents) {
                            const auto maxExtent = std::max(std::max(xExtent, yExtent), zExtent);
                            const std::array<TypeParam, 3> aabbCenter{ { 4 * maxExtent, 0, 0 } };
                            const auto obb = dense_mapping::Geometry::orientedBoundingBox(center, rotation, xExtent, yExtent, zExtent);

                            EXPECT_TRUE(dense_mapping::Geometry::details::allOBBCornersInNegativeHalfSpace(obb, aabbCenter, TypeParam(1)))
                                << "Theta = [" << theta << " / " << thetaIdx << "], phi = [" << phi << " / " << phiIdx << "], alpha = ["
                                << alpha << "], xE [" << xExtent << "], yE [" << yExtent << "], zE [" << zExtent << "]";
                        }
                    }
                }
            }
        }
    }
}

TYPED_TEST(ObbBoxIntersectionTest, allAabbCornersWithinObbNegativeHalfSpace) {
    constexpr TypeParam extents[] = { 1, 10, 100, 10000, 10000, 100000, 1000000, 10000000 };

    constexpr size_t thetaSamples = 30;

    constexpr size_t phiSamples = 15;

    constexpr TypeParam rotationMagnitudes[] = { 0, 1 * M_PI / 180, 5 * M_PI / 180, 15 * M_PI / 180, 45 * M_PI / 180, 85 * M_PI / 180,
        135 * M_PI / 180, 165 * M_PI / 180, 175 * M_PI / 180 };

    constexpr std::array<TypeParam, 3> center{ { 0, 0, 0 } };

    // Sample the unit sphere for a rotation axis
    for (size_t thetaIdx = 0; thetaIdx < thetaSamples; ++thetaIdx) {
        const TypeParam theta = 2 * M_PI * thetaIdx / thetaSamples - M_PI;
        const auto st = std::sin(theta);
        const auto ct = std::cos(theta);

        for (size_t phiIdx = 0; phiIdx < phiSamples; ++phiIdx) {
            const TypeParam phi = M_PI * phiIdx / phiSamples - M_PI / 2;
            const auto sp = std::sin(phi);
            const auto cp = std::cos(phi);

            // Sample rotation amount
            for (const auto alpha : rotationMagnitudes) {
                const std::array<TypeParam, 3> rotation{ { alpha * st * cp, alpha * st * sp, alpha * ct } };

                // Sample extents for the obb
                for (const auto xExtent : extents) {
                    for (const auto yExtent : extents) {
                        for (const auto zExtent : extents) {
                            const auto maxExtent = std::max(std::max(xExtent, yExtent), zExtent);
                            const std::array<TypeParam, 3> aabbCenter{ { -5 * maxExtent, 0, 0 } };
                            const auto obb = dense_mapping::Geometry::orientedBoundingBox(center, rotation, xExtent, yExtent, zExtent);

                            EXPECT_TRUE(dense_mapping::Geometry::details::allBoxCornersInNegativeHalfSpace(obb, aabbCenter, TypeParam(1)))
                                << "Theta = [" << theta << " / " << thetaIdx << "], phi = [" << phi << " / " << phiIdx << "], alpha = ["
                                << alpha << "], xE [" << xExtent << "], yE [" << yExtent << "], zE [" << zExtent << "]";
                        }
                    }
                }
            }
        }
    }
}

TYPED_TEST(ObbBoxIntersectionTest, definitelyDisjoint) {
    constexpr TypeParam extents[] = { 1, 10, 100, 10000, 10000, 100000, 1000000, 10000000 };

    constexpr size_t thetaSamples = 10;

    constexpr size_t phiSamples = 5;

    constexpr TypeParam rotationMagnitudes[] = { 0, 1 * M_PI / 180, 5 * M_PI / 180, 15 * M_PI / 180, 45 * M_PI / 180, 85 * M_PI / 180,
        135 * M_PI / 180, 165 * M_PI / 180, 175 * M_PI / 180 };

    constexpr std::array<TypeParam, 3> center{ { 0, 0, 0 } };

    // Sample the unit sphere for a rotation axis
    for (size_t thetaIdx1 = 0; thetaIdx1 < thetaSamples; ++thetaIdx1) {
        const TypeParam theta1 = 2 * M_PI * thetaIdx1 / thetaSamples - M_PI;
        const auto st1 = std::sin(theta1);
        const auto ct1 = std::cos(theta1);

        for (size_t phiIdx1 = 0; phiIdx1 < phiSamples; ++phiIdx1) {
            const TypeParam phi1 = M_PI * phiIdx1 / phiSamples - M_PI / 2;
            const auto sp1 = std::sin(phi1);
            const auto cp1 = std::cos(phi1);

            // Sample rotation amount
            for (const auto alpha1 : rotationMagnitudes) {
                const std::array<TypeParam, 3> rotation{ { alpha1 * st1 * cp1, alpha1 * st1 * sp1, alpha1 * ct1 } };

                for (const auto xExtent : extents) {
                    for (const auto yExtent : extents) {
                        for (const auto zExtent : extents) {
                            const auto maxExtent = std::max(std::max(xExtent, yExtent), zExtent);

                            // Sample the unit sphere for a second axis
                            for (size_t thetaIdx2 = 0; thetaIdx2 < thetaSamples; ++thetaIdx2) {
                                const TypeParam theta2 = 2 * M_PI * thetaIdx2 / thetaSamples - M_PI;
                                const auto st2 = std::sin(theta2);
                                const auto ct2 = std::cos(theta2);

                                for (size_t phiIdx2 = 0; phiIdx2 < phiSamples; ++phiIdx2) {
                                    const TypeParam phi2 = M_PI * phiIdx2 / phiSamples - M_PI / 2;
                                    const auto sp2 = std::sin(phi2);
                                    const auto cp2 = std::cos(phi2);

                                    const std::array<TypeParam, 3> aabbCenter{ { 5 * maxExtent * st2 * cp2, 5 * maxExtent * st2 * sp2,
                                        5 * maxExtent * ct2 } };
                                    const auto obb
                                        = dense_mapping::Geometry::orientedBoundingBox(aabbCenter, rotation, xExtent, yExtent, zExtent);
                                    EXPECT_FALSE(dense_mapping::Geometry::obbIntersectsAxisAlignedBoundingBox(obb, center, TypeParam(1)));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
}
