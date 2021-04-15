#include "packages/math/geometry/quaternion.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

using namespace geometry;

TEST(Geometry, QuaternionToRodriguesIdempotent_RawFloat) {
    float q[4] = { 0.967676, -0.049460, 0.000000, -0.247300 }; // w,x,y,z
    float r[3];
    convertQuaternionToRodrigues(r[0], r[1], r[2], q[0], q[1], q[2], q[3]);

    float rExpected[3] = { -0.09999981, 0, -0.499999 };

    EXPECT_NEAR(r[0], rExpected[0], 1e-5);
    EXPECT_NEAR(r[1], rExpected[1], 1e-5);
    EXPECT_NEAR(r[2], rExpected[2], 1e-5);
}

TEST(Geometry, QuaternionToRodriguesIdempotent_EigenMatrix) {
    float q[4] = { 0.967676, -0.049460, 0.000000, -0.247300 }; // w,x,y,z
    auto r = convertQuaternionToRodrigues(q[0], q[1], q[2], q[3]);

    float rExpected[3] = { -0.09999981, 0, -0.499999 };

    EXPECT_NEAR(r(0, 0), rExpected[0], 1e-5);
    EXPECT_NEAR(r(1, 0), rExpected[1], 1e-5);
    EXPECT_NEAR(r(2, 0), rExpected[2], 1e-5);
}

TEST(Geometry, RodriguesToQuaternionIdempotent_RawFloat) {
    float r[3] = { -0.09999981, 0, -0.499999 };
    float q[4];
    convertRodriguesToQuaternion(q[0], q[1], q[2], q[3], r[0], r[1], r[2]);

    float qExpected[4] = { 0.967676, -0.049460, 0.000000, -0.247300 }; // w,x,y,z

    EXPECT_NEAR(q[0], qExpected[0], 1e-5);
    EXPECT_NEAR(q[1], qExpected[1], 1e-5);
    EXPECT_NEAR(q[2], qExpected[2], 1e-5);
    EXPECT_NEAR(q[3], qExpected[3], 1e-5);
}

TEST(Geometry, RodriguesToQuaternionIdempotent_Quaternion) {
    float r[3] = { -0.09999981, 0, -0.499999 };

    auto q = convertRodriguesToQuaternion(r[0], r[1], r[2]);

    float qExpected[4] = { 0.967676, -0.049460, 0.000000, -0.247300 }; // w,x,y,z

    EXPECT_NEAR(q.w(), qExpected[0], 1e-5);
    EXPECT_NEAR(q.x(), qExpected[1], 1e-5);
    EXPECT_NEAR(q.y(), qExpected[2], 1e-5);
    EXPECT_NEAR(q.z(), qExpected[3], 1e-5);
}
