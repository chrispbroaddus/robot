#include "../include/box_geometry.h"
#include "gtest/gtest.h"

namespace {
template <typename T> struct FrustumTest : public ::testing::Test {};

using TypesToTest = ::testing::Types<float, double>;

TYPED_TEST_CASE(FrustumTest, TypesToTest);

TYPED_TEST(FrustumTest, sanityCheckFrusum) {
    constexpr auto epsilon = 100 * std::numeric_limits<TypeParam>::epsilon();

    constexpr std::array<TypeParam, 3> frustumCenter{ { 0, 0, 0 } };
    constexpr std::array<TypeParam, 3> zero{ { 0, 0, 0 } };

    for (int alphaDeg = 1; alphaDeg <= 90; ++alphaDeg) {
        // Horizontal half FOV
        const TypeParam alpha = alphaDeg * M_PI / 180;

        for (int betaDeg = 1; betaDeg <= 90; ++betaDeg) {
            const TypeParam beta = betaDeg * M_PI / 180;

            const auto frustum = dense_mapping::Geometry::frustum(frustumCenter, zero, alpha, beta);

            EXPECT_EQ(frustum.m_origin[0], frustumCenter[0]);
            EXPECT_EQ(frustum.m_origin[1], frustumCenter[1]);
            EXPECT_EQ(frustum.m_origin[2], frustumCenter[2]);

            EXPECT_NEAR(1, dense_mapping::Geometry::normL2(frustum.m_topPlaneNormal), epsilon)
                << "Normal: [" << frustum.m_topPlaneNormal[0] << ", " << frustum.m_topPlaneNormal[1] << ", " << frustum.m_topPlaneNormal[2]
                << "]";
            EXPECT_NEAR(1, dense_mapping::Geometry::normL2(frustum.m_bottomPlaneNormal), epsilon)
                << "Normal: [" << frustum.m_bottomPlaneNormal[0] << ", " << frustum.m_bottomPlaneNormal[1] << ", "
                << frustum.m_bottomPlaneNormal[2] << "]";
            EXPECT_NEAR(1, dense_mapping::Geometry::normL2(frustum.m_leftPlaneNormal), epsilon)
                << "Normal: [" << frustum.m_leftPlaneNormal[0] << ", " << frustum.m_leftPlaneNormal[1] << ", "
                << frustum.m_leftPlaneNormal[2] << "]";
            EXPECT_NEAR(1, dense_mapping::Geometry::normL2(frustum.m_rightPlaneNormal), epsilon)
                << "Normal: [" << frustum.m_rightPlaneNormal[0] << ", " << frustum.m_rightPlaneNormal[1] << ", "
                << frustum.m_rightPlaneNormal[2] << "]";

            for (int zStep = -100; zStep <= 100; ++zStep) {
                const std::array<TypeParam, 3> pt{ { 0, 0, static_cast<TypeParam>(zStep) } };
                const std::array<TypeParam, 3> vpt{ { pt[0] - frustumCenter[0], pt[1] - frustumCenter[1], pt[2] - frustumCenter[2] } };

                if (zStep < 0) {
                    EXPECT_GT(0, dense_mapping::Geometry::innerProduct(vpt, frustum.m_topPlaneNormal))
                        << " zS = " << zStep << ", vpt=[" << vpt[0] << ", " << vpt[1] << ", " << vpt[2] << "] @ alpha = " << alphaDeg
                        << ", beta = " << betaDeg << ", top = [" << frustum.m_topPlaneNormal[0] << ", " << frustum.m_topPlaneNormal[1]
                        << ", " << frustum.m_topPlaneNormal[2] << "]";
                    EXPECT_GT(0, dense_mapping::Geometry::innerProduct(vpt, frustum.m_bottomPlaneNormal))
                        << " zS = " << zStep << ", vpt=[" << vpt[0] << ", " << vpt[1] << ", " << vpt[2] << "] @ alpha = " << alphaDeg
                        << ", beta = " << betaDeg << ", bottom = [" << frustum.m_bottomPlaneNormal[0] << ", "
                        << frustum.m_bottomPlaneNormal[1] << ", " << frustum.m_bottomPlaneNormal[2] << "]";
                    EXPECT_GT(0, dense_mapping::Geometry::innerProduct(vpt, frustum.m_leftPlaneNormal))
                        << " zS = " << zStep << ", vpt=[" << vpt[0] << ", " << vpt[1] << ", " << vpt[2] << "] @ alpha = " << alphaDeg
                        << ", beta = " << betaDeg << ", left = [" << frustum.m_leftPlaneNormal[0] << ", " << frustum.m_leftPlaneNormal[1]
                        << ", " << frustum.m_leftPlaneNormal[2] << "]";
                    EXPECT_GT(0, dense_mapping::Geometry::innerProduct(vpt, frustum.m_rightPlaneNormal))
                        << " zS = " << zStep << ", vpt=[" << vpt[0] << ", " << vpt[1] << ", " << vpt[2] << "] @ alpha = " << alphaDeg
                        << ", beta = " << betaDeg << ", right = [" << frustum.m_rightPlaneNormal[0] << ", " << frustum.m_rightPlaneNormal[1]
                        << ", " << frustum.m_rightPlaneNormal[2] << "]";

                } else if (zStep > 0) {
                    EXPECT_LT(0, dense_mapping::Geometry::innerProduct(vpt, frustum.m_topPlaneNormal))
                        << " zS = " << zStep << ", vpt=[" << vpt[0] << ", " << vpt[1] << ", " << vpt[2] << "] @ alpha = " << alphaDeg
                        << ", beta = " << betaDeg;
                    EXPECT_LT(0, dense_mapping::Geometry::innerProduct(vpt, frustum.m_bottomPlaneNormal))
                        << " zS = " << zStep << ", vpt=[" << vpt[0] << ", " << vpt[1] << ", " << vpt[2] << "] @ alpha = " << alphaDeg
                        << ", beta = " << betaDeg;
                    EXPECT_LT(0, dense_mapping::Geometry::innerProduct(vpt, frustum.m_leftPlaneNormal))
                        << " zS = " << zStep << ", vpt=[" << vpt[0] << ", " << vpt[1] << ", " << vpt[2] << "] @ alpha = " << alphaDeg
                        << ", beta = " << betaDeg;
                    EXPECT_LT(0, dense_mapping::Geometry::innerProduct(vpt, frustum.m_rightPlaneNormal))
                        << " zS = " << zStep << ", vpt=[" << vpt[0] << ", " << vpt[1] << ", " << vpt[2] << "] @ alpha = " << alphaDeg
                        << ", beta = " << betaDeg;
                } else {
                    EXPECT_NEAR(0, dense_mapping::Geometry::innerProduct(vpt, frustum.m_topPlaneNormal), epsilon)
                        << " zS = " << zStep << ", vpt=[" << vpt[0] << ", " << vpt[1] << ", " << vpt[2] << "] @ alpha = " << alphaDeg
                        << ", beta = " << betaDeg;
                    EXPECT_NEAR(0, dense_mapping::Geometry::innerProduct(vpt, frustum.m_bottomPlaneNormal), epsilon)
                        << " zS = " << zStep << ", vpt=[" << vpt[0] << ", " << vpt[1] << ", " << vpt[2] << "] @ alpha = " << alphaDeg
                        << ", beta = " << betaDeg;
                    EXPECT_NEAR(0, dense_mapping::Geometry::innerProduct(vpt, frustum.m_leftPlaneNormal), epsilon)
                        << " zS = " << zStep << ", vpt=[" << vpt[0] << ", " << vpt[1] << ", " << vpt[2] << "] @ alpha = " << alphaDeg
                        << ", beta = " << betaDeg;
                    EXPECT_NEAR(0, dense_mapping::Geometry::innerProduct(vpt, frustum.m_rightPlaneNormal), epsilon)
                        << " zS = " << zStep << ", vpt=[" << vpt[0] << ", " << vpt[1] << ", " << vpt[2] << "] @ alpha = " << alphaDeg
                        << ", beta = " << betaDeg;
                }
            }
        }
    }
}
}