#include "packages/triangulation/include/triangulation.h"

#include "gtest/gtest.h"

using namespace triangulation;

TEST(triangulateDirectional, trivialRegressionTest) {
    const Eigen::Matrix<double, 3, 3> R0 = Eigen::Matrix<double, 3, 3>::Identity();
    const Eigen::Matrix<double, 3, 1> t0 = { -0.5, 0, 0 };
    const Eigen::Matrix<double, 3, 3> R1 = Eigen::Matrix<double, 3, 3>::Identity();
    const Eigen::Matrix<double, 3, 1> t1 = { 0.5, 0, 0 };

    Eigen::Matrix<double, 3, 3> projR0;
    Eigen::Matrix<double, 3, 1> projt0;
    invertRigidTransform(projR0, projt0, R0, t0);

    Eigen::Matrix<double, 3, 3> projR1;
    Eigen::Matrix<double, 3, 1> projt1;
    invertRigidTransform(projR1, projt1, R1, t1);

    const Eigen::Matrix<double, 3, 1> M = { 0, 0, 10 };
    const Eigen::Matrix<double, 2, 1> m0 = (projR0 * M + projt0).hnormalized();
    const Eigen::Matrix<double, 2, 1> m1 = (projR1 * M + projt1).hnormalized();

    Eigen::Matrix<double, 3, 1> Mp;
    EXPECT_TRUE(triangulateDirectional(Mp, R0, t0, m0, R1, t1, m1));

    EXPECT_DOUBLE_EQ(M(0), Mp(0));
    EXPECT_DOUBLE_EQ(M(1), Mp(1));
    EXPECT_DOUBLE_EQ(M(2), Mp(2));
}

TEST(triangulateDirectional, colocatedCameras) {
    const Eigen::Matrix<double, 3, 3> R0 = Eigen::Matrix<double, 3, 3>::Identity();
    const Eigen::Matrix<double, 3, 1> t0 = { 0, 0, 0 };
    const Eigen::Matrix<double, 3, 3> R1 = Eigen::Matrix<double, 3, 3>::Identity();
    const Eigen::Matrix<double, 3, 1> t1 = { 0, 0, 0 };

    Eigen::Matrix<double, 3, 3> projR0;
    Eigen::Matrix<double, 3, 1> projt0;
    invertRigidTransform(projR0, projt0, R0, t0);

    Eigen::Matrix<double, 3, 3> projR1;
    Eigen::Matrix<double, 3, 1> projt1;
    invertRigidTransform(projR1, projt1, R1, t1);

    const Eigen::Matrix<double, 3, 1> M = { 0, 0, 10 };
    const Eigen::Matrix<double, 2, 1> m0 = (projR0 * M + projt0).hnormalized();
    const Eigen::Matrix<double, 2, 1> m1 = (projR1 * M + projt1).hnormalized();

    Eigen::Matrix<double, 3, 1> Mp;
    EXPECT_FALSE(triangulateDirectional(Mp, R0, t0, m0, R1, t1, m1));
}

TEST(triangulateDirectional, parallelRays) {
    const Eigen::Matrix<double, 3, 3> R0 = Eigen::Matrix<double, 3, 3>::Identity();
    const Eigen::Matrix<double, 3, 1> t0 = { 0, 0, 0 };
    const Eigen::Matrix<double, 3, 3> R1 = Eigen::Matrix<double, 3, 3>::Identity();
    const Eigen::Matrix<double, 3, 1> t1 = { 1, 0, 0 };

    const Eigen::Matrix<double, 2, 1> m0 = { 0, 0 };
    const Eigen::Matrix<double, 2, 1> m1 = { 0, 0 };

    Eigen::Matrix<double, 3, 1> Mp;
    EXPECT_FALSE(triangulateDirectional(Mp, R0, t0, m0, R1, t1, m1));
}

TEST(triangulateDirectional, regressionTest) {
    const Eigen::Matrix<double, 3, 3> R0 = Eigen::Matrix<double, 3, 3>::Identity();
    const Eigen::Matrix<double, 3, 1> t0 = { -0.5, 0.5, -0.5 };
    const Eigen::Matrix<double, 3, 3> R1 = Eigen::Matrix<double, 3, 3>::Identity();
    const Eigen::Matrix<double, 3, 1> t1 = { 0.5, -0.5, 0.5 };

    Eigen::Matrix<double, 3, 3> projR0;
    Eigen::Matrix<double, 3, 1> projt0;
    invertRigidTransform(projR0, projt0, R0, t0);

    Eigen::Matrix<double, 3, 3> projR1;
    Eigen::Matrix<double, 3, 1> projt1;
    invertRigidTransform(projR1, projt1, R1, t1);

    const Eigen::Matrix<double, 3, 1> M = { -0.5, 0.5, 100 };
    const Eigen::Matrix<double, 2, 1> m0 = (projR0 * M + projt0).hnormalized();
    const Eigen::Matrix<double, 2, 1> m1 = (projR1 * M + projt1).hnormalized();

    Eigen::Matrix<double, 3, 1> Mp;
    EXPECT_TRUE(triangulateDirectional(Mp, R0, t0, m0, R1, t1, m1));

    EXPECT_NEAR(M(0), Mp(0), 1e-12);
    EXPECT_NEAR(M(1), Mp(1), 1e-12);
    EXPECT_NEAR(M(2), Mp(2), 1e-12);
}