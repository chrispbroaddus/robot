
#pragma once

#include <stdint.h>
#include <string>

namespace hal {

class DeviceInterface {
public:
    virtual std::string deviceName() const = 0;
    virtual uint64_t serialNumber() const = 0;
};
}
