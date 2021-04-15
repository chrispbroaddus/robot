#include "packages/hal/include/drivers/imus/unity_simulated/unity_simulated_imu_device_factory.h"

namespace hal {

std::shared_ptr<ImuDeviceInterface> UnitySimulatedImuDeviceFactory::create(const hal::details::property_map_t& config) {

    uint32_t imuId;

    auto iter = config.find("imuId");
    if (iter != config.end()) {
        imuId = (uint32_t)std::stoul(iter->second);
    } else {
        LOG(ERROR) << "Missing property: imuId";
        throw std::runtime_error("Missing property: imuId");
    }

    LOG(INFO) << "Unity simulated IMU being created";
    return std::make_shared<UnitySimulatedImuDevice>(imuId, config);
}
}
