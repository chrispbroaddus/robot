#pragma once

#include "packages/hal/proto/network_health_telemetry.pb.h"

namespace hal {
namespace details {
    ::google::protobuf::Map<std::string, hal::NetworkInterfaceStatistics> getInterfaceStatistics();
}
}
