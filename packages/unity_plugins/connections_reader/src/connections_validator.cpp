#include "glog/logging.h"
#include "packages/unity_plugins/connections_reader/include/connections_reader.h"

using namespace unity_plugins;

///
/// @brief Read in a calibration file and check it can be parsed
///
int main(int argc, char** argv) {

    google::InitGoogleLogging(argv[0]);

    if (argc != 2) {
        LOG(ERROR) << "USAGE: " << argv[0] << " CONNECTIONS_FILE_NAME";
        return EXIT_FAILURE;
    }

    std::string filename(argv[1]);

    ConnectionsReader reader(filename);

    auto connections = reader.connections();

    return EXIT_SUCCESS;
}