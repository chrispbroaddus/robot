
#pragma once

#include "glog/logging.h"
#include "packages/hal/include/hal_types.h"

#include <map>
#include <memory>

namespace hal {

template <typename DEVICE_CLASS_T> class DeviceFactory;

template <typename DEVICE_CLASS_T> class DeviceRegistry {
public:
    typedef DEVICE_CLASS_T device_class_t;

    static DeviceRegistry<device_class_t>& instance() {
        static DeviceRegistry<device_class_t> s_instance;
        return s_instance;
    }

    void registerFactory(const std::string& factoryName, DeviceFactory<device_class_t>* factory) {
        LOG(INFO) << "Registering factory: " << factoryName;
        m_factories[factoryName] = factory;
    }

    std::shared_ptr<device_class_t> create(const hal::details::property_map_t& config);

    void logRegisteredFactories() const {
        LOG(INFO) << "Registered factories";
        for (const auto& factory : m_factories) {
            LOG(INFO) << factory.first;
        }
    }

private:
    DeviceRegistry() = default;
    ~DeviceRegistry() = default;

    std::map<std::string, DeviceFactory<device_class_t>*> m_factories;
};
}
