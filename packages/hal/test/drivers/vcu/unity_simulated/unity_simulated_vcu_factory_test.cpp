#include "packages/hal/include/drivers/vcu/unity_simulated/unity_simulated_vcu_factory.h"
#include "gtest/gtest.h"

using namespace hal::vcu;

TEST(HalUnitySimulatedVcuFactoryTest, canInstantiate) { EXPECT_NO_THROW(UnitySimulatedVcuFactory unitySimulatedVcuFactory); }

TEST(HalUnitySimulatedVcuFactoryTest, canCreateVcu) {

    UnitySimulatedVcuFactory unitySimulatedVcuFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["repServerAddress"] = "tcp://localhost:5561";
    deviceConfig["lingerPeriodInMilliseconds"] = "1000";
    deviceConfig["sendRecvTimeoutInMilliseconds"] = "500";
    deviceConfig["subscriberAddress"] = "tcp://localhost:5562";
    deviceConfig["subscriberTopic"] = "telemetry";
    deviceConfig["subscriberHighWaterMark"] = "100";

    EXPECT_NO_THROW(unitySimulatedVcuFactory.create(deviceConfig));
}
