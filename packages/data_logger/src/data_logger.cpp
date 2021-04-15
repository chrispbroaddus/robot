
#include "packages/data_logger/include/data_logger.h"
#include "glog/logging.h"
#include "packages/data_logger/include/data_logger_graph.h"
#include "packages/hal/include/device_factory.h"
#include "packages/hal/include/string_utils.h"

#include <fcntl.h>
#include <fstream>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>
#include <iostream>
#include <unistd.h>

using namespace data_logger;

static const std::string s_descriptionFileName = "STREAM_DESCRIPTION";

static std::string getPrettyDataFileString() {
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[256];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", timeinfo);
    return std::string(buffer);
}

DataLogger::DataLogger(const std::string& configFile, const std::string& systemCalibrationFile, const std::string& serialNumber,
    const std::string& dataOutputDir, const bool displayVideoStreams)
    : m_configFile(configFile)
    , m_dataOutputDir(dataOutputDir)
    , m_systemCalibrationFile(systemCalibrationFile)
    , m_serialNumber(serialNumber) {

    m_sessionName = getPrettyDataFileString();
    m_sessionPath = m_dataOutputDir + "/" + m_sessionName;

    LOG(INFO) << "Data logger using config file: " << configFile;

    std::ifstream inputFileStream(configFile, std::ios::in);
    if (!inputFileStream.is_open()) {
        throw std::runtime_error("DataLogger unable to open config file");
    }

    google::protobuf::io::IstreamInputStream inputStream(&inputFileStream);

    DataLoggerConfig config;
    if (!google::protobuf::TextFormat::Parse(&inputStream, &config)) {
        throw std::runtime_error("DataLogger failed to parse config file");
    }

    m_dataLoggerGraph = std::unique_ptr<DataLoggerGraph>(new DataLoggerGraph(config, m_sessionPath, displayVideoStreams));

    createStreamDescriptionFile(config);
    copySystemCalibrationFile();
    createSerialNumberFile();
}

DataLogger::~DataLogger() {}

void DataLogger::start() { m_dataLoggerGraph->start(); }

void DataLogger::stop() { m_dataLoggerGraph->stop(); }

void DataLogger::createStreamDescriptionFile(const data_logger::DataLoggerConfig& dataLoggerConfig) {
    std::string streamDescriptionString;
    if (!google::protobuf::TextFormat::PrintToString(dataLoggerConfig, &streamDescriptionString)) {
        throw std::runtime_error("DataLogger unable to get stream description");
    }

    const std::string outputFilePath = m_sessionPath + "/" + s_descriptionFileName;
    std::ofstream outputFile(outputFilePath, std::ofstream::out);
    if (!outputFile.is_open()) {
        throw std::runtime_error("DataLogger unable to open stream description file");
    }

    outputFile << streamDescriptionString;
    outputFile.close();
}

void DataLogger::copySystemCalibrationFile() {

    const std::string outputFilePath = m_sessionPath + "/SystemCalibrationFile.json";
    std::ifstream inputFile(m_systemCalibrationFile, std::ifstream::in);
    std::ofstream outputFile(outputFilePath, std::ofstream::out);
    if (!outputFile.is_open()) {
        throw std::runtime_error("DataLogger unable to open system calibration file");
    }

    outputFile << inputFile.rdbuf();
    outputFile.close();
    inputFile.close();
}

void DataLogger::createSerialNumberFile() {

    const std::string outputFilePath = m_sessionPath + "/serialNumber.txt";
    std::ofstream outputFile(outputFilePath, std::ofstream::out);
    if (!outputFile.is_open()) {
        throw std::runtime_error("DataLogger unable to open serial number file");
    }

    outputFile << m_serialNumber;
    outputFile.close();
}

namespace data_logger {

bool createExampleConfigFile(const std::string& outputPath) {
    LOG(INFO) << "Creating data logger example configuration file: " << outputPath;

    DataLoggerConfig config;

    data_logger::Stream* camera0 = config.add_camera();
    camera0->set_name("camera0");
    camera0->set_server_address("tcp://localhost:5556");
    camera0->set_topic("camera");

    data_logger::Stream* camera1 = config.add_camera();
    camera1->set_name("camera1");
    camera1->set_server_address("tcp://localhost:5557");
    camera1->set_topic("camera");

    config.mutable_jpeg_encoder_options()->set_quality(80);

    std::string outputExampleString;
    if (!google::protobuf::TextFormat::PrintToString(config, &outputExampleString)) {
        throw std::runtime_error("DataLogger unable to create string from protobuf");
    }

    std::ofstream outputFile(outputPath, std::ofstream::out);
    if (!outputFile.is_open()) {
        throw std::runtime_error("DataLogger unable to open file");
    }

    outputFile << outputExampleString;
    outputFile.close();

    return true;
}
}
