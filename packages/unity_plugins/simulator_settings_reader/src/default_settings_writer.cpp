#include "packages/unity_plugins/simulator_settings_reader/include/default_simulator_settings.h"
#include <fstream>
#include <glog/logging.h>
#include <google/protobuf/util/json_util.h>
#include <string>

///
/// Write out the default simulator settings file
///
int main(int argc, char** argv) {

    google::InitGoogleLogging(argv[0]);

    simulator_settings::SimulatorSettings settings = unity_plugins::defaultSimulatorSettings();

    std::string msg;
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    google::protobuf::util::MessageToJsonString(settings, &msg, options);

    std::ofstream out("default_simulator_settings.json");
    out << msg;
    out.close();
}