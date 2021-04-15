#include "glog/logging.h"
#include "gtest/gtest.h"

#include "packages/unity_plugins/utils/include/coordinate_conversion.h"

using namespace unity_plugins;

TEST(UnityToZippy, TranslationOnlyCheckIdempotence) {
    float rxp, ryp, rzp, txp, typ, tzp;
    float rx = 0;
    float ry = 0;
    float rz = 0;
    float tx = 1;
    float ty = 0;
    float tz = 0;
    unity_plugins::convertUnityVehicleToZippyVehicleCoordinate(rxp, ryp, rzp, txp, typ, tzp, rx, ry, rz, tx, ty, tz);

    EXPECT_NEAR(rxp, 0, 1e-4);
    EXPECT_NEAR(ryp, 0, 1e-4);
    EXPECT_NEAR(rzp, 0, 1e-4);

    EXPECT_NEAR(txp, 0, 1e-4);
    EXPECT_NEAR(typ, 1, 1e-4);
    EXPECT_NEAR(tzp, 0, 1e-4);

    // Check round-trip conversion
    float rxpp, rypp, rzpp, txpp, typp, tzpp;
    unity_plugins::convertZippyVehicleToUnityVehicleCoordinate(rxpp, rypp, rzpp, txpp, typp, tzpp, rxp, ryp, rzp, txp, typ, tzp);

    EXPECT_EQ(rx, rxpp);
    EXPECT_EQ(ry, rypp);
    EXPECT_EQ(rz, rzpp);

    EXPECT_EQ(tx, txpp);
    EXPECT_EQ(ty, typp);
    EXPECT_EQ(tz, tzpp);
}

TEST(UnityToZippy, TranslationAndRotationCheckIdempotence) {
    float rxp, ryp, rzp, txp, typ, tzp;

    float rx = -M_PI / 2;
    float ry = 0;
    float rz = 0;
    float tx = 1;
    float ty = 0;
    float tz = 0;
    unity_plugins::convertUnityVehicleToZippyVehicleCoordinate(rxp, ryp, rzp, txp, typ, tzp, rx, ry, rz, tx, ty, tz);

    EXPECT_EQ(rxp, 0);
    EXPECT_NEAR(ryp, M_PI / 2, 1e-6);
    EXPECT_EQ(rzp, 0);

    EXPECT_EQ(txp, 0);
    EXPECT_EQ(typ, 1);
    EXPECT_EQ(tzp, 0);

    // Check round-trip conversion
    float rxpp, rypp, rzpp, txpp, typp, tzpp;
    unity_plugins::convertZippyVehicleToUnityVehicleCoordinate(rxpp, rypp, rzpp, txpp, typp, tzpp, rxp, ryp, rzp, txp, typ, tzp);

    EXPECT_EQ(rx, rxpp);
    EXPECT_EQ(ry, rypp);
    EXPECT_EQ(rz, rzpp);

    EXPECT_EQ(tx, txpp);
    EXPECT_EQ(ty, typp);
    EXPECT_EQ(tz, tzpp);
}
