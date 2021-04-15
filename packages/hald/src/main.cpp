#include "gflags/gflags.h"
#include "glog/logging.h"

#include "packages/hald/include/hald_application.h"

DEFINE_string(configFile, "", "Configuration file");

int main(int argc, char* argv[]) {

    google::InitGoogleLogging(argv[0]);

    gflags::SetUsageMessage("hald");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    if (argc < 2) {
        gflags::ShowUsageWithFlagsRestrict(argv[0], "");
        return 0;
    }

    auto config = hald::parseConfig(FLAGS_configFile);
    hald::HaldApplication haldApplication(config);

    while (haldApplication.isRunning()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}