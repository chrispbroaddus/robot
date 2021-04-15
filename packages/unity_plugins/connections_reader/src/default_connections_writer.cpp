#include "packages/unity_plugins/connections_reader/include/default_simulator_connections.h"
#include <fstream>
#include <glog/logging.h>
#include <google/protobuf/util/json_util.h>
#include <string>

///
/// Write out the default simulator connections file
///
int main(int argc, char** argv) {

    mercury::Connections connections = unity_plugins::defaultSimulatorConnections();

    std::string msg;
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    google::protobuf::util::MessageToJsonString(connections, &msg, options);

    std::ofstream out("default_simulator_connections.json");
    out << msg;
    out.close();
}
