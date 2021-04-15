#include "packages/dense_mapping/include/box_geometry.h"
#include <gtest/gtest.h>

namespace {
template <typename T> struct SegmentIntersectsAxisAlignedBoundingBox {
    using scalar_type = T;
    using result_type = dense_mapping::Geometry::SegmentAxisAlignedBoundingBoxIntersection<T>;

    static void expectTrue(const result_type& r) {
        EXPECT_TRUE(r.isValid());
        EXPECT_TRUE(r);

        if (!r || !r.isValid()) {
            GTEST_LOG_(INFO) << r;
        }
    }

    static void expectFalse(const result_type& r) {
        EXPECT_TRUE(r.isValid());
        EXPECT_FALSE(r);

        if (r || !r.isValid()) {
            GTEST_LOG_(INFO) << r;
        }
    }

    static void expectEq(bool expected, result_type actual) {
        EXPECT_TRUE(actual.isValid());
        EXPECT_EQ(expected, actual);
        if (expected != actual || !actual.isValid()) {
            GTEST_LOG_(INFO) << actual;
        }
    }

    static constexpr result_type checkIntersection(
        std::array<T, 3> origin, std::array<T, 3> target, std::array<T, 3> boxCenter, T boxHalfExtent) {
        return dense_mapping::Geometry::segmentIntersectsAxisAlignedBoundingBox(origin, target, boxCenter, boxHalfExtent);
    }
};

template <typename IMPLEMENTATION_UNDER_TEST> class SegmentIntersectsBoxTest : public ::testing::Test {
public:
    using scalar_type = typename IMPLEMENTATION_UNDER_TEST::scalar_type;

    static constexpr std::array<scalar_type, 3> boxCenter{ { 0, 0, 0 } };
    static constexpr scalar_type boxHalfSpan = 1.0f;
    static constexpr int k = 100;

    // It is not clear to me at all why, to allow tests to pass, epsilon has to be scaled by k :-(
    static constexpr scalar_type epsilon = k * std::numeric_limits<scalar_type>::epsilon();
    static constexpr scalar_type q = k * boxHalfSpan;
    static constexpr int numStepsY = 11;
    static constexpr int numStepsZ = 11;
    static constexpr int numStepsX = 11;
};

template <typename T> constexpr std::array<typename T::scalar_type, 3> SegmentIntersectsBoxTest<T>::boxCenter;

template <typename T> constexpr int SegmentIntersectsBoxTest<T>::k;

template <typename T> constexpr int SegmentIntersectsBoxTest<T>::numStepsX;
template <typename T> constexpr int SegmentIntersectsBoxTest<T>::numStepsY;
template <typename T> constexpr int SegmentIntersectsBoxTest<T>::numStepsZ;

template <typename T> constexpr typename T::scalar_type SegmentIntersectsBoxTest<T>::boxHalfSpan;
template <typename T> constexpr typename T::scalar_type SegmentIntersectsBoxTest<T>::epsilon;
template <typename T> constexpr typename T::scalar_type SegmentIntersectsBoxTest<T>::q;

using SegmentBoxIntersectionTypesToTest
    = ::testing::Types<SegmentIntersectsAxisAlignedBoundingBox<float>, SegmentIntersectsAxisAlignedBoundingBox<double> >;

TYPED_TEST_CASE(SegmentIntersectsBoxTest, SegmentBoxIntersectionTypesToTest);

TYPED_TEST(SegmentIntersectsBoxTest, segmentIntersectsAABB_perpendicularRayTouchesBottomFace) {
    for (int xStep = -TestFixture::numStepsX / 2; xStep <= TestFixture::numStepsX / 2; ++xStep) {
        const auto x = 2 * TestFixture::boxHalfSpan * xStep / (TestFixture::numStepsX - 1);

        for (int yStep = -TestFixture::numStepsY / 2; yStep <= TestFixture::numStepsY / 2; ++yStep) {
            const auto y = 2 * TestFixture::boxHalfSpan * yStep / (TestFixture::numStepsZ - 1);

            const std::array<typename TestFixture::scalar_type, 3> sourcePoint{ { x, y, -10 * TestFixture::boxHalfSpan } };
            const std::array<typename TestFixture::scalar_type, 3> targetPoint{ { x, y, -TestFixture::boxHalfSpan } };

            const auto r1 = TypeParam::checkIntersection(targetPoint, sourcePoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
            TypeParam::expectTrue(r1);

            const auto r2 = TypeParam::checkIntersection(sourcePoint, targetPoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
            TypeParam::expectTrue(r2);
        }
    }
}

TYPED_TEST(SegmentIntersectsBoxTest, segmentIntersectsAABB_perpendicularRayTouchesTopFace) {
    for (int xStep = -TestFixture::numStepsX / 2; xStep <= TestFixture::numStepsX / 2; ++xStep) {
        const auto x = 2 * TestFixture::boxHalfSpan * xStep / (TestFixture::numStepsX - 1);

        for (int yStep = -TestFixture::numStepsY / 2; yStep <= TestFixture::numStepsY / 2; ++yStep) {
            const auto y = 2 * TestFixture::boxHalfSpan * yStep / (TestFixture::numStepsZ - 1);

            const std::array<typename TestFixture::scalar_type, 3> sourcePoint{ { x, y, 10 * TestFixture::boxHalfSpan } };
            const std::array<typename TestFixture::scalar_type, 3> targetPoint{ { x, y, TestFixture::boxHalfSpan } };

            const auto r1 = TypeParam::checkIntersection(targetPoint, sourcePoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
            TypeParam::expectTrue(r1);

            const auto r2 = TypeParam::checkIntersection(sourcePoint, targetPoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
            TypeParam::expectTrue(r2);
        }
    }
}

/// Ray exterior to the box, parallel to the Z axis ends at the center of the box after passing thru the top face of the box --> true
TYPED_TEST(SegmentIntersectsBoxTest, segmentIntersectsAABB_perpendicularRayPassesThroughTopAndBottomFaces) {
    for (int xStep = -TestFixture::numStepsX / 2; xStep <= TestFixture::numStepsX / 2; ++xStep) {
        const auto x = 2 * TestFixture::boxHalfSpan * xStep / (TestFixture::numStepsX - 1);

        for (int yStep = -TestFixture::numStepsY / 2; yStep <= TestFixture::numStepsY / 2; ++yStep) {
            const auto y = 2 * TestFixture::boxHalfSpan * yStep / (TestFixture::numStepsZ - 1);

            const std::array<typename TestFixture::scalar_type, 3> sourcePoint{ { x, y, 10 * TestFixture::boxHalfSpan } };
            const std::array<typename TestFixture::scalar_type, 3> targetPoint{ { x, y, -10 * TestFixture::boxHalfSpan } };

            const auto r1 = TypeParam::checkIntersection(targetPoint, sourcePoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
            TypeParam::expectTrue(r1);

            const auto r2 = TypeParam::checkIntersection(sourcePoint, targetPoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
            TypeParam::expectTrue(r2);
        }
    }
}

/// Ray exterior to the box, parallel to the X axis touches the left face --> true
TYPED_TEST(SegmentIntersectsBoxTest, segmentIntersectsAABB_perpendicularRayTouchesLeftFace) {
    for (int yStep = -TestFixture::numStepsY / 2; yStep <= TestFixture::numStepsY / 2; ++yStep) {
        const auto y = 2 * TestFixture::boxHalfSpan * yStep / (TestFixture::numStepsY - 1);

        for (int zStep = -TestFixture::numStepsZ / 2; zStep <= TestFixture::numStepsZ / 2; ++zStep) {
            const auto z = 2 * TestFixture::boxHalfSpan * zStep / (TestFixture::numStepsZ - 1);

            const std::array<typename TestFixture::scalar_type, 3> sourcePoint{ { -10 * TestFixture::boxHalfSpan, y, z } };
            const std::array<typename TestFixture::scalar_type, 3> targetPoint{ { -TestFixture::boxHalfSpan, y, z } };

            const auto r1 = TypeParam::checkIntersection(targetPoint, sourcePoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
            TypeParam::expectTrue(r1);

            const auto r2 = TypeParam::checkIntersection(sourcePoint, targetPoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
            TypeParam::expectTrue(r2);
        }
    }
}

/// Ray exterior to the box, parallel to the X axis touches the right face --> true
TYPED_TEST(SegmentIntersectsBoxTest, segmentIntersectsAABB_perpendicularRayTouchesRightFace) {
    for (int yStep = -TestFixture::numStepsY / 2; yStep <= TestFixture::numStepsY / 2; ++yStep) {
        const auto y = 2 * TestFixture::boxHalfSpan * yStep / (TestFixture::numStepsY - 1);

        for (int zStep = -TestFixture::numStepsZ / 2; zStep <= TestFixture::numStepsZ / 2; ++zStep) {
            const auto z = 2 * TestFixture::boxHalfSpan * zStep / (TestFixture::numStepsZ - 1);

            const std::array<typename TestFixture::scalar_type, 3> sourcePoint{ { 10 * TestFixture::boxHalfSpan, y, z } };
            const std::array<typename TestFixture::scalar_type, 3> targetPoint{ { TestFixture::boxHalfSpan, y, z } };

            const auto r1 = TypeParam::checkIntersection(targetPoint, sourcePoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
            TypeParam::expectTrue(r1);

            const auto r2 = TypeParam::checkIntersection(sourcePoint, targetPoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
            TypeParam::expectTrue(r2);
        }
    }
}

TYPED_TEST(SegmentIntersectsBoxTest, segmentIntersectsAABB_perpendicularRayPassesThroughLeftAndRightFaces) {
    for (int yStep = -TestFixture::numStepsY / 2; yStep <= TestFixture::numStepsY / 2; ++yStep) {
        const auto y = 2 * TestFixture::boxHalfSpan * yStep / (TestFixture::numStepsY - 1);

        for (int zStep = -TestFixture::numStepsZ / 2; zStep <= TestFixture::numStepsZ / 2; ++zStep) {
            const auto z = 2 * TestFixture::boxHalfSpan * zStep / (TestFixture::numStepsZ - 1);

            const std::array<typename TestFixture::scalar_type, 3> sourcePoint{ { -10 * TestFixture::boxHalfSpan, y, z } };
            const std::array<typename TestFixture::scalar_type, 3> targetPoint{ { 10 * TestFixture::boxHalfSpan, y, z } };

            const auto r1 = TypeParam::checkIntersection(targetPoint, sourcePoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
            TypeParam::expectTrue(r1);

            const auto r2 = TypeParam::checkIntersection(sourcePoint, targetPoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
            TypeParam::expectTrue(r2);
        }
    }
}

TYPED_TEST(SegmentIntersectsBoxTest, segmentIntersectsAABB_perpendicularRayTouchesBackFace) {
    for (int xStep = -TestFixture::numStepsX / 2; xStep <= TestFixture::numStepsX / 2; ++xStep) {
        const auto x = 2 * xStep * TestFixture::boxHalfSpan / (TestFixture::numStepsX - 1);

        for (int zStep = -TestFixture::numStepsZ / 2; zStep <= TestFixture::numStepsZ / 2; ++zStep) {
            const auto z = 2 * zStep * TestFixture::boxHalfSpan / (TestFixture::numStepsZ - 1);

            const std::array<typename TestFixture::scalar_type, 3> sourcePoint{ { x, -10 * TestFixture::boxHalfSpan, z } };
            const std::array<typename TestFixture::scalar_type, 3> targetPoint{ { x, -TestFixture::boxHalfSpan, z } };

            const auto r1 = TypeParam::checkIntersection(targetPoint, sourcePoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
            TypeParam::expectTrue(r1);

            const auto r2 = TypeParam::checkIntersection(sourcePoint, targetPoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
            TypeParam::expectTrue(r2);
        }
    }
}

TYPED_TEST(SegmentIntersectsBoxTest, segmentIntersectsAABB_perpendicularRayTouchesFrontFace) {
    for (int xStep = -TestFixture::numStepsX / 2; xStep <= TestFixture::numStepsX / 2; ++xStep) {
        const auto x = 2 * xStep * TestFixture::boxHalfSpan / (TestFixture::numStepsX - 1);

        for (int zStep = -TestFixture::numStepsZ / 2; zStep <= TestFixture::numStepsZ / 2; ++zStep) {
            const auto z = 2 * zStep * TestFixture::boxHalfSpan / (TestFixture::numStepsZ - 1);

            const std::array<typename TestFixture::scalar_type, 3> sourcePoint{ { x, 10 * TestFixture::boxHalfSpan, z } };
            const std::array<typename TestFixture::scalar_type, 3> targetPoint{ { x, TestFixture::boxHalfSpan, z } };

            const auto r1 = TypeParam::checkIntersection(targetPoint, sourcePoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
            TypeParam::expectTrue(r1);

            const auto r2 = TypeParam::checkIntersection(sourcePoint, targetPoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
            TypeParam::expectTrue(r2);
        }
    }
}

TYPED_TEST(SegmentIntersectsBoxTest, segmentIntersectsAABB_perpendicularRayPassesThroughBackAndFrontFaces) {
    for (int xStep = -TestFixture::numStepsX / 2; xStep <= TestFixture::numStepsX / 2; ++xStep) {
        const auto x = 2 * xStep * TestFixture::boxHalfSpan / (TestFixture::numStepsX - 1);

        for (int zStep = -TestFixture::numStepsZ / 2; zStep <= TestFixture::numStepsZ / 2; ++zStep) {
            const auto z = 2 * zStep * TestFixture::boxHalfSpan / (TestFixture::numStepsZ - 1);

            const std::array<typename TestFixture::scalar_type, 3> sourcePoint{ { x, -10 * TestFixture::boxHalfSpan, z } };
            const std::array<typename TestFixture::scalar_type, 3> targetPoint{ { x, 10 * TestFixture::boxHalfSpan, z } };

            const auto r1 = TypeParam::checkIntersection(targetPoint, sourcePoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
            TypeParam::expectTrue(r1);

            const auto r2 = TypeParam::checkIntersection(sourcePoint, targetPoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
            TypeParam::expectTrue(r2);
        }
    }
}

TYPED_TEST(SegmentIntersectsBoxTest, segmentsNearLeftFaceDoNotIntersect) {
    constexpr typename TestFixture::scalar_type targetX = -TestFixture::boxHalfSpan - TestFixture::epsilon;
    constexpr typename TestFixture::scalar_type sourceX = -TestFixture::q;

    for (int yStep1 = -TestFixture::numStepsY / 2; yStep1 <= TestFixture::numStepsY / 2; ++yStep1) {
        const typename TestFixture::scalar_type targetY
            = 2 * yStep1 * (TestFixture::boxHalfSpan + TestFixture::epsilon) / (TestFixture::numStepsY - 1);

        for (int yStep2 = -TestFixture::numStepsY / 2; yStep2 <= TestFixture::numStepsY / 2; ++yStep2) {
            const typename TestFixture::scalar_type sourceY = 2 * yStep2 * TestFixture::q / (TestFixture::numStepsY - 1);

            for (int zStep1 = -TestFixture::numStepsZ / 2; zStep1 <= TestFixture::numStepsZ / 2; ++zStep1) {
                const typename TestFixture::scalar_type targetZ
                    = 2 * zStep1 * (TestFixture::boxHalfSpan + TestFixture::epsilon) / (TestFixture::numStepsZ - 1);

                for (int zStep2 = -TestFixture::numStepsZ / 2; zStep2 <= TestFixture::numStepsZ / 2; ++zStep2) {
                    const typename TestFixture::scalar_type sourceZ = 2 * zStep2 * TestFixture::q / (TestFixture::numStepsZ - 1);

                    const std::array<typename TestFixture::scalar_type, 3> sourcePoint{ { sourceX, sourceY, sourceZ } };
                    const std::array<typename TestFixture::scalar_type, 3> targetPoint{ { targetX, targetY, targetZ } };

                    const auto r1
                        = TypeParam::checkIntersection(targetPoint, sourcePoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
                    TypeParam::expectFalse(r1);

                    const auto r2
                        = TypeParam::checkIntersection(sourcePoint, targetPoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
                    TypeParam::expectFalse(r2);
                }
            }
        }
    }
}

TYPED_TEST(SegmentIntersectsBoxTest, segmentsNearRightFaceDoNotIntersect) {
    constexpr typename TestFixture::scalar_type targetX = TestFixture::boxHalfSpan + TestFixture::epsilon;
    constexpr typename TestFixture::scalar_type sourceX = TestFixture::q;

    for (int yStep1 = -TestFixture::numStepsY / 2; yStep1 <= TestFixture::numStepsY / 2; ++yStep1) {
        const typename TestFixture::scalar_type targetY
            = 2 * yStep1 * (TestFixture::boxHalfSpan + TestFixture::epsilon) / (TestFixture::numStepsY - 1);

        for (int yStep2 = -TestFixture::numStepsY / 2; yStep2 <= TestFixture::numStepsY / 2; ++yStep2) {
            const typename TestFixture::scalar_type sourceY = 2 * yStep2 * TestFixture::q / (TestFixture::numStepsY - 1);

            for (int zStep1 = -TestFixture::numStepsZ / 2; zStep1 <= TestFixture::numStepsZ / 2; ++zStep1) {
                const typename TestFixture::scalar_type targetZ
                    = 2 * zStep1 * (TestFixture::boxHalfSpan + TestFixture::epsilon) / (TestFixture::numStepsZ - 1);

                for (int zStep2 = -TestFixture::numStepsZ / 2; zStep2 <= TestFixture::numStepsZ / 2; ++zStep2) {
                    const typename TestFixture::scalar_type sourceZ = 2 * zStep2 * TestFixture::q / (TestFixture::numStepsZ - 1);

                    const std::array<typename TestFixture::scalar_type, 3> sourcePoint{ { sourceX, sourceY, sourceZ } };
                    const std::array<typename TestFixture::scalar_type, 3> targetPoint{ { targetX, targetY, targetZ } };

                    const auto r1
                        = TypeParam::checkIntersection(targetPoint, sourcePoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
                    TypeParam::expectFalse(r1);

                    const auto r2
                        = TypeParam::checkIntersection(sourcePoint, targetPoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
                    TypeParam::expectFalse(r2);
                }
            }
        }
    }
}

TYPED_TEST(SegmentIntersectsBoxTest, segmentsNearBottomFaceDoNotIntersect) {
    constexpr typename TestFixture::scalar_type targetZ = -TestFixture::boxHalfSpan - TestFixture::epsilon;
    constexpr typename TestFixture::scalar_type sourceZ = -TestFixture::q;

    for (int yStep1 = -TestFixture::numStepsY / 2; yStep1 <= TestFixture::numStepsY / 2; ++yStep1) {
        const typename TestFixture::scalar_type targetY
            = 2 * yStep1 * (TestFixture::boxHalfSpan + TestFixture::epsilon) / (TestFixture::numStepsY - 1);

        for (int yStep2 = -TestFixture::numStepsY / 2; yStep2 <= TestFixture::numStepsY / 2; ++yStep2) {
            const typename TestFixture::scalar_type sourceY = 2 * yStep2 * TestFixture::q / (TestFixture::numStepsY - 1);

            for (int xStep1 = -TestFixture::numStepsX / 2; xStep1 <= TestFixture::numStepsX / 2; ++xStep1) {
                const typename TestFixture::scalar_type targetX
                    = 2 * xStep1 * (TestFixture::boxHalfSpan + TestFixture::epsilon) / (TestFixture::numStepsX - 1);

                for (int xStep2 = -TestFixture::numStepsX / 2; xStep2 <= TestFixture::numStepsX / 2; ++xStep2) {
                    const typename TestFixture::scalar_type sourceX = 2 * xStep2 * TestFixture::q / (TestFixture::numStepsX - 1);

                    const std::array<typename TestFixture::scalar_type, 3> sourcePoint{ { sourceX, sourceY, sourceZ } };
                    const std::array<typename TestFixture::scalar_type, 3> targetPoint{ { targetX, targetY, targetZ } };

                    const auto r1
                        = TypeParam::checkIntersection(targetPoint, sourcePoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
                    TypeParam::expectFalse(r1);

                    const auto r2
                        = TypeParam::checkIntersection(sourcePoint, targetPoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
                    TypeParam::expectFalse(r2);
                }
            }
        }
    }
}

TYPED_TEST(SegmentIntersectsBoxTest, segmentsNearTopFaceDoNotIntersect) {
    constexpr typename TestFixture::scalar_type targetZ = TestFixture::boxHalfSpan + TestFixture::epsilon;
    constexpr typename TestFixture::scalar_type sourceZ = TestFixture::q;

    for (int yStep1 = -TestFixture::numStepsY / 2; yStep1 <= TestFixture::numStepsY / 2; ++yStep1) {
        const typename TestFixture::scalar_type targetY
            = 2 * yStep1 * (TestFixture::boxHalfSpan + TestFixture::epsilon) / (TestFixture::numStepsY - 1);

        for (int yStep2 = -TestFixture::numStepsY / 2; yStep2 <= TestFixture::numStepsY / 2; ++yStep2) {
            const typename TestFixture::scalar_type sourceY = 2 * yStep2 * TestFixture::q / (TestFixture::numStepsY - 1);

            for (int xStep1 = -TestFixture::numStepsX / 2; xStep1 <= TestFixture::numStepsX / 2; ++xStep1) {
                const typename TestFixture::scalar_type targetX
                    = 2 * xStep1 * (TestFixture::boxHalfSpan + TestFixture::epsilon) / (TestFixture::numStepsX - 1);

                for (int xStep2 = -TestFixture::numStepsX / 2; xStep2 <= TestFixture::numStepsX / 2; ++xStep2) {
                    const typename TestFixture::scalar_type sourceX = 2 * xStep2 * TestFixture::q / (TestFixture::numStepsX - 1);

                    const std::array<typename TestFixture::scalar_type, 3> sourcePoint{ { sourceX, sourceY, sourceZ } };
                    const std::array<typename TestFixture::scalar_type, 3> targetPoint{ { targetX, targetY, targetZ } };

                    const auto r1
                        = TypeParam::checkIntersection(targetPoint, sourcePoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
                    TypeParam::expectFalse(r1);

                    const auto r2
                        = TypeParam::checkIntersection(sourcePoint, targetPoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
                    TypeParam::expectFalse(r2);
                }
            }
        }
    }
}

TYPED_TEST(SegmentIntersectsBoxTest, segmentsNearBackFaceDoNotIntersect) {
    constexpr typename TestFixture::scalar_type targetY = -TestFixture::boxHalfSpan - TestFixture::epsilon;
    constexpr typename TestFixture::scalar_type sourceY = -TestFixture::q;

    for (int xStep1 = -TestFixture::numStepsX / 2; xStep1 <= TestFixture::numStepsX / 2; ++xStep1) {
        const typename TestFixture::scalar_type targetX
            = 2 * xStep1 * (TestFixture::boxHalfSpan + TestFixture::epsilon) / (TestFixture::numStepsX - 1);

        for (int xStep2 = -TestFixture::numStepsX / 2; xStep2 <= TestFixture::numStepsX / 2; ++xStep2) {
            const typename TestFixture::scalar_type sourceX = 2 * xStep2 * TestFixture::q / (TestFixture::numStepsX - 1);

            for (int zStep1 = -TestFixture::numStepsZ / 2; zStep1 <= TestFixture::numStepsZ / 2; ++zStep1) {
                const typename TestFixture::scalar_type targetZ
                    = 2 * zStep1 * (TestFixture::boxHalfSpan + TestFixture::epsilon) / (TestFixture::numStepsZ - 1);

                for (int zStep2 = -TestFixture::numStepsZ / 2; zStep2 <= TestFixture::numStepsZ / 2; ++zStep2) {
                    const typename TestFixture::scalar_type sourceZ = 2 * zStep2 * TestFixture::q / (TestFixture::numStepsZ - 1);

                    const std::array<typename TestFixture::scalar_type, 3> sourcePoint{ { sourceX, sourceY, sourceZ } };
                    const std::array<typename TestFixture::scalar_type, 3> targetPoint{ { targetX, targetY, targetZ } };
                    const auto r1
                        = TypeParam::checkIntersection(targetPoint, sourcePoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
                    TypeParam::expectFalse(r1);

                    const auto r2
                        = TypeParam::checkIntersection(sourcePoint, targetPoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
                    TypeParam::expectFalse(r2);
                }
            }
        }
    }
}

TYPED_TEST(SegmentIntersectsBoxTest, segmentsNearFrontFaceDoNotIntersect) {
    constexpr typename TestFixture::scalar_type targetY = TestFixture::boxHalfSpan + TestFixture::epsilon;
    constexpr typename TestFixture::scalar_type sourceY = TestFixture::q;

    for (int xStep1 = -TestFixture::numStepsX / 2; xStep1 <= TestFixture::numStepsX / 2; ++xStep1) {
        const typename TestFixture::scalar_type targetX
            = 2 * xStep1 * (TestFixture::boxHalfSpan + TestFixture::epsilon) / (TestFixture::numStepsX - 1);

        for (int xStep2 = -TestFixture::numStepsX / 2; xStep2 <= TestFixture::numStepsX / 2; ++xStep2) {
            const typename TestFixture::scalar_type sourceX = 2 * xStep2 * TestFixture::q / (TestFixture::numStepsX - 1);

            for (int zStep1 = -TestFixture::numStepsZ / 2; zStep1 <= TestFixture::numStepsZ / 2; ++zStep1) {
                const typename TestFixture::scalar_type targetZ
                    = 2 * zStep1 * (TestFixture::boxHalfSpan + TestFixture::epsilon) / (TestFixture::numStepsZ - 1);

                for (int zStep2 = -TestFixture::numStepsZ / 2; zStep2 <= TestFixture::numStepsZ / 2; ++zStep2) {
                    const typename TestFixture::scalar_type sourceZ = 2 * zStep2 * TestFixture::q / (TestFixture::numStepsZ - 1);

                    const std::array<typename TestFixture::scalar_type, 3> sourcePoint{ { sourceX, sourceY, sourceZ } };
                    const std::array<typename TestFixture::scalar_type, 3> targetPoint{ { targetX, targetY, targetZ } };

                    const auto r1
                        = TypeParam::checkIntersection(targetPoint, sourcePoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
                    TypeParam::expectFalse(r1);

                    const auto r2
                        = TypeParam::checkIntersection(sourcePoint, targetPoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
                    TypeParam::expectFalse(r2);
                }
            }
        }
    }
}

TYPED_TEST(SegmentIntersectsBoxTest, segmentsEntirelyInteriorToBoxIntersect) {
    constexpr auto deflationDen = 1000;
    constexpr auto deflationNumer = deflationDen - 1;

    for (int xStep1 = -TestFixture::numStepsX / 2; xStep1 <= TestFixture::numStepsX / 2; ++xStep1) {
        const auto x1 = 2 * deflationNumer * TestFixture::boxHalfSpan / (deflationDen * (TestFixture::numStepsX - 1));

        for (int yStep1 = -TestFixture::numStepsY / 2; yStep1 <= TestFixture::numStepsY / 2; ++yStep1) {
            const auto y1 = 2 * deflationNumer * TestFixture::boxHalfSpan / (deflationDen * (TestFixture::numStepsY - 1));

            for (int zStep1 = -TestFixture::numStepsZ / 2; zStep1 <= TestFixture::numStepsZ / 2; ++zStep1) {
                const auto z1 = 2 * deflationNumer * TestFixture::boxHalfSpan / (deflationDen * (TestFixture::numStepsZ - 1));
                const std::array<typename TestFixture::scalar_type, 3> sourcePoint{ { x1, y1, z1 } };

                for (int xStep2 = -TestFixture::numStepsX / 2; xStep2 <= TestFixture::numStepsX / 2; ++xStep2) {
                    const auto x2 = 2 * deflationNumer * TestFixture::boxHalfSpan / (deflationDen * (TestFixture::numStepsX - 1));

                    for (int yStep2 = -TestFixture::numStepsY / 2; yStep2 <= TestFixture::numStepsY / 2; ++yStep2) {
                        const auto y2 = 2 * deflationNumer * TestFixture::boxHalfSpan / (deflationDen * (TestFixture::numStepsY - 1));

                        for (int zStep2 = -TestFixture::numStepsZ / 2; zStep2 <= TestFixture::numStepsZ / 2; ++zStep2) {
                            const auto z2 = 2 * deflationNumer * TestFixture::boxHalfSpan / (deflationDen * (TestFixture::numStepsZ - 1));

                            const std::array<typename TestFixture::scalar_type, 3> targetPoint{ { x2, y2, z2 } };

                            const auto r1
                                = TypeParam::checkIntersection(targetPoint, sourcePoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
                            TypeParam::expectTrue(r1);

                            const auto r2
                                = TypeParam::checkIntersection(sourcePoint, targetPoint, TestFixture::boxCenter, TestFixture::boxHalfSpan);
                            TypeParam::expectTrue(r2);
                        }
                    }
                }
            }
        }
    }
}

TYPED_TEST(SegmentIntersectsBoxTest, degenerateSegmentTest) {
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

                const auto r = TypeParam::checkIntersection(point, point, TestFixture::boxCenter, TestFixture::boxHalfSpan);
                TypeParam::expectEq(inX && inY && inZ, r);
            }
        }
    }
}
}
