
#include "gflags/gflags.h"
#include "glog/logging.h"

#include "packages/data_logger/include/data_logger.h"
#include "packages/hal/include/factory_registration.h"

#include <iostream>
#include <signal.h>
#include <thread>

DEFINE_string(configFile, "", "Configuration file");
DEFINE_string(systemCalibrationFile, "", "System calibration file for the zippy generating the data");
DEFINE_string(serialNumber, "", "Serial number of the zippy generating the data");
DEFINE_bool(exampleConfig, false, "Create an example configuration file");
DEFINE_string(exampleConfigFile, "/tmp/data_logger_config_example.txt", "Path to write example config");
DEFINE_string(dataOutputDir, "", "Output data directory");
DEFINE_bool(display, false, "Display video streams");

static bool s_stopDataLogger = false;

void SIGINTHandler(int) {
    LOG(INFO) << "Shutting down data logger";
    s_stopDataLogger = true;
}

int main(int argc, char* argv[]) {

    signal(SIGINT, SIGINTHandler);

    google::InitGoogleLogging(argv[0]);

    hal::Initialize();

    gflags::SetUsageMessage("Data collection for cameras, imus and other sensors");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    if (!FLAGS_exampleConfig
        && (FLAGS_configFile.empty() || FLAGS_systemCalibrationFile.empty() || FLAGS_serialNumber.empty() || FLAGS_dataOutputDir.empty())) {
        gflags::ShowUsageWithFlagsRestrict(argv[0], "");
        return 0;
    }

    if (FLAGS_exampleConfig) {
        data_logger::createExampleConfigFile(FLAGS_exampleConfigFile);
        return 0;
    }

    data_logger::DataLogger dataLogger(
        FLAGS_configFile, FLAGS_systemCalibrationFile, FLAGS_serialNumber, FLAGS_dataOutputDir, FLAGS_display);

    dataLogger.start();

    while (!s_stopDataLogger) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    dataLogger.stop();

    gflags::ShutDownCommandLineFlags();

    return 0;
}
