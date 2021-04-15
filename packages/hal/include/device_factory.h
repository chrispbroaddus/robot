
#pragma once

#include "packages/hal/include/device_registry.h"
#include "packages/hal/include/hal_types.h"

namespace hal {

namespace details {
    static const std::string FACTORY_NAME_KEY = "factory";
}

template <typename DEVICE_CLASS_T> class DeviceFactory {
public:
    typedef DEVICE_CLASS_T device_class_t;

    DeviceFactory() = delete;
    DeviceFactory(const std::string& factoryName)
        : m_factoryName(factoryName) {
        DeviceRegistry<device_class_t>::instance().registerFactory(factoryName, this);
    }
    ~DeviceFactory() = default;

    const std::string& factoryName() const { return m_factoryName; }

    virtual std::shared_ptr<device_class_t> create(const details::property_map_t& config) = 0;

private:
    std::string m_factoryName;
};
}
