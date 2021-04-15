#include "packages/hal/include/drivers/vcu/unity_simulated/unity_simulated_vcu_factory.h"
#include "packages/hal/include/string_utils.h"

#include <iostream>

namespace hal {

namespace vcu {

    std::shared_ptr<VCUDeviceInterface> UnitySimulatedVcuFactory::create(const hal::details::property_map_t& config) {

        LOG(INFO) << "Unity Simulated VCU being created";

        return std::make_shared<UnitySimulatedVcu>(config);
    }
} // vcu
} // hal