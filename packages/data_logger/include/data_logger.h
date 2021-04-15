
#pragma once

#include "packages/data_logger/proto/config.pb.h"

#include <memory>

namespace data_logger {

class DataLoggerGraph;

class DataLogger {
public:
    DataLogger(const std::string& configFile, const std::string& systemCalibrationFile, const std::string& serialNumber,
        const std::string& dataOutputDir, const bool displayVideoStreams);
    ~DataLogger();
    DataLogger(const DataLogger&) = delete;
    DataLogger(const DataLogger&&) = delete;
    DataLogger& operator=(const DataLogger&) = delete;
    DataLogger& operator=(const DataLogger&&) = delete;

    void start();
    void stop();

private:
    const std::string m_configFile;
    const std::string m_dataOutputDir;
    const std::string m_systemCalibrationFile;
    const std::string m_serialNumber;
    std::string m_sessionName;
    std::string m_sessionPath;

    std::unique_ptr<DataLoggerGraph> m_dataLoggerGraph;

    void createStreamDescriptionFile(const data_logger::DataLoggerConfig& dataLoggerConfig);
    void copySystemCalibrationFile();
    void createSerialNumberFile();
};

bool createExampleConfigFile(const std::string& outputPath);
}
