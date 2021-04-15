#include "packages/unity_plugins/simulator_settings_reader/include/simulator_settings_reader.h"
#include "glog/logging.h"
#include "packages/unity_plugins/utils/include/zippy_interface.h"
#include <memory>

using namespace unity_plugins;

static std::unique_ptr<SimulatorSettingsReader> s_reader;

extern "C" {

///
/// \brief Load the simulator settings from a single json file to construct SimulatorSettingsReader
/// \return 1 : Success, 0 : Fail
///
ZIPPY_INTERFACE_EXPORT int SimulatorSettingsReader_Load(SimulatorSettings* settings, const char* filename) {
    std::string filenameStr(filename);
    try {
        s_reader.reset(new SimulatorSettingsReader(filenameStr));
        memcpy(settings, &s_reader->interopSettings(), sizeof(SimulatorSettings));
        return 1;
    } catch (const std::exception& e) {
        LOG(ERROR) << e.what();
        return 0;
    }
}
}