#pragma once

#include "packages/unity_plugins/simulator_settings_reader/include/simulator_settings_reader_interop.h"
#include "packages/unity_plugins/utils/include/zippy_image_interop.h"
#include "packages/unity_simulator/proto/simulator_settings.pb.h"
#include <map>
#include <string>

namespace unity_plugins {

///
/// Read simulation settings from ProtoBuf-JSON file,
///
class SimulatorSettingsReader {

public:
    SimulatorSettingsReader(const std::string& filename);
    ~SimulatorSettingsReader() = default;
    SimulatorSettingsReader(const SimulatorSettingsReader& obj) = delete;
    SimulatorSettingsReader& operator=(SimulatorSettingsReader other) = delete;

    /// settings in probuffer format
    const simulator_settings::SimulatorSettings& settings() { return m_settings; }

    /// settings in C# compatible structs
    const SimulatorSettings& interopSettings() { return m_interopSettings; }

private:
    simulator_settings::SimulatorSettings m_settings;
    SimulatorSettings m_interopSettings;
};
} // namespace unity_plugins
