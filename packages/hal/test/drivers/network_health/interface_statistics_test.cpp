#include "packages/hal/include/drivers/network_health/interface_statistics.h"

#include "google/protobuf/text_format.h"

#include "glog/logging.h"
#include "gtest/gtest.h"

TEST(InterfaceStatisticsTest, canQueryStatistics) {
    auto statistics = hal::details::getInterfaceStatistics();

    for (auto i = statistics.begin(); i != statistics.end(); ++i) {
        std::string formatted;
        ::google::protobuf::TextFormat::PrintToString(i->second, &formatted);
        LOG(INFO) << "INTERFACE [" << i->first << "] STATISTICS [" << formatted << "]";
    }
}