#include "packages/hal/include/drivers/network_health/network_health_driver_factory.h"
#include <algorithm>

#include "packages/hal/include/drivers/network_health/network_health_driver.h"

namespace hal {
std::shared_ptr<NetworkHealthDriverInterface> NetworkHealthDriverFactory::create(const details::property_map_t& config) {
    std::vector<std::string> targetHosts;
    std::transform(config.begin(), config.end(), std::back_inserter(targetHosts),
        [](const std::pair<std::string, std::string>& x) { return x.first; });
    targetHosts.erase(std::remove(targetHosts.begin(), targetHosts.end(), "factory"), targetHosts.end());
    return std::make_shared<NetworkHealthDriver>(targetHosts);
}
}