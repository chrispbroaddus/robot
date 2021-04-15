#include "packages/filesystem/include/filesystem.h"
#include "packages/image_codec/include/image_decoder_interface.h"
#include "packages/serialization/include/protobuf_io.h"

#include "packages/data_logger/proto/config.pb.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/hal/proto/gps_telemetry.pb.h"
#include "packages/hal/proto/joystick_sample.pb.h"
#include "packages/hal/proto/network_health_telemetry.pb.h"
#include "packages/hal/proto/vcu_telemetry_envelope.pb.h"

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "src/google/protobuf/text_format.h"

#include <algorithm>
#include <fstream>
#include <typeinfo>

/// Structure used to compute stats of interest for a sensor stream
struct ProtodatStats {
    ProtodatStats() {
        prevId = -1;
        droppedSamples = 0;
        startTime = 0;
        endTime = 0;
    }
    std::map<std::string, uint64_t> loggedSamplesMap;
    int64_t prevId;
    uint32_t droppedSamples;
    uint64_t startTime;
    uint64_t endTime;
};

DEFINE_string(inputDir, "", "path to the directory containing the logged data");

/// Takes a protobuf message as input and updates the relevant stats
template <typename MSG_T>
void updateStats(const std::unique_ptr<google::protobuf::Message>& messagePtr, struct ProtodatStats& protodatStats) {

    if (typeid(MSG_T) == typeid(hal::CameraSample)) {
        protodatStats.loggedSamplesMap["CameraSamples"]++;
        hal::CameraSample* message = dynamic_cast<hal::CameraSample*>(messagePtr.get());
        if (protodatStats.prevId != -1 && message->id() != protodatStats.prevId + 1) {
            protodatStats.droppedSamples += message->id() - protodatStats.prevId - 1;
        }
        if (protodatStats.startTime == 0) {
            protodatStats.startTime = message->systemtimestamp().nanos();
        }
        protodatStats.prevId = message->id();
        protodatStats.endTime = message->systemtimestamp().nanos();
    } else if (typeid(MSG_T) == typeid(hal::GPSTelemetry)) {
        protodatStats.loggedSamplesMap["GpsSamples"]++;
        hal::GPSTelemetry* message = dynamic_cast<hal::GPSTelemetry*>(messagePtr.get());
        if (protodatStats.startTime == 0) {
            protodatStats.startTime = message->timestamp().nanos();
        }
        protodatStats.endTime = message->timestamp().nanos();
    } else if (typeid(MSG_T) == typeid(hal::JoystickSample)) {
        protodatStats.loggedSamplesMap["JoystickSamples"]++;
        hal::JoystickSample* message = dynamic_cast<hal::JoystickSample*>(messagePtr.get());
        if (protodatStats.startTime == 0) {
            protodatStats.startTime = message->systemtimestamp().nanos();
        }
        protodatStats.endTime = message->systemtimestamp().nanos();
    } else if (typeid(MSG_T) == typeid(hal::NetworkHealthTelemetry)) {
        protodatStats.loggedSamplesMap["NetworkHealthSamples"]++;
        hal::NetworkHealthTelemetry* message = dynamic_cast<hal::NetworkHealthTelemetry*>(messagePtr.get());
        if (protodatStats.startTime == 0) {
            protodatStats.startTime = message->measurement_start_system_timestamp().nanos();
        }
        protodatStats.endTime = message->measurement_end_system_timestamp().nanos();
    } else if (typeid(MSG_T) == typeid(hal::VCUTelemetryEnvelope)) {
        protodatStats.loggedSamplesMap["VCUTelemetrySamples"]++;
        hal::VCUTelemetryEnvelope* message = dynamic_cast<hal::VCUTelemetryEnvelope*>(messagePtr.get());
        if (message->has_servo()) {
            protodatStats.loggedSamplesMap["ServoTelemetrySamples"]++;
        } else if (message->has_slider()) {
            protodatStats.loggedSamplesMap["SliderTelemetrySamples"]++;
        } else if (message->has_suspensionencoder()) {
            protodatStats.loggedSamplesMap["SuspensionEncoderSamples"]++;
        } else if (message->has_wheelencoder()) {
            protodatStats.loggedSamplesMap["WheelEncoderSamples"]++;
        }
        if (protodatStats.startTime == 0) {
            protodatStats.startTime = message->sendtimestamp().nanos();
        }
        protodatStats.endTime = message->sendtimestamp().nanos();
    }
}

/// Compute the stats of interest for a sensor stream
template <typename MSG_T> struct ProtodatStats getProtodatStats(std::string protodatFolderPath) {

    struct ProtodatStats protodatStats;
    std::unique_ptr<serialization::ProtobufReader> protobufReader;
    std::unique_ptr<google::protobuf::Message> messagePtr;
    messagePtr.reset(new MSG_T);
    std::ifstream inputFileStream;
    std::vector<std::string> fileList = filesystem::getFileList(protodatFolderPath);
    std::vector<std::string> filteredFileList;

    for (auto& file : fileList) {
        if (file.size() >= 9 && file.substr(file.size() - 9) == ".protodat") {
            filteredFileList.push_back(protodatFolderPath + file);
        }
    }
    std::sort(filteredFileList.begin(), filteredFileList.end());

    for (auto& file : filteredFileList) {

        inputFileStream.open(file, std::ios::in | std::ios::binary);
        if (!inputFileStream.is_open()) {
            throw std::runtime_error("Unable to open protodat file: " + file);
        }

        protobufReader = std::unique_ptr<serialization::ProtobufReader>(new serialization::ProtobufReader(&inputFileStream));
        while (protobufReader.get()) {
            if (protobufReader->readNext(*dynamic_cast<MSG_T*>(messagePtr.get()))) {
                updateStats<MSG_T>(messagePtr, protodatStats);
            } else {
                break;
            }
        }
        inputFileStream.close();
    }

    return protodatStats;
}

/// Prints the computed stats
void logProtodatStats(const struct ProtodatStats& protodatStats) {
    for (const auto& pair : protodatStats.loggedSamplesMap) {
        LOG(INFO) << "Logged samples of type: " << pair.first << " : " << pair.second;
    }
    if (protodatStats.prevId != -1) {
        LOG(INFO) << "Dropped Samples: " << protodatStats.droppedSamples;
    }
    LOG(INFO) << "Measurement duration: " << (protodatStats.endTime - protodatStats.startTime) / 1.0e9 << " seconds";
}

/// Utility to compute simple statistics on the data captured by the data_logger
int main(int argc, char* argv[]) {

    gflags::SetUsageMessage("Get statistics from the data logged by the datalogger");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    if (FLAGS_inputDir.empty()) {
        gflags::ShowUsageWithFlagsRestrict(argv[0], "");
        return 0;
    }

    std::vector<std::string> fileList = filesystem::getFileList(FLAGS_inputDir);

    std::string baseFolderPath;
    baseFolderPath = FLAGS_inputDir.substr(FLAGS_inputDir.size() - 1) == "/" ? FLAGS_inputDir : FLAGS_inputDir + "/";

    std::string streamDescriptionFile;
    for (auto& file : fileList) {
        if (file == "STREAM_DESCRIPTION") {
            streamDescriptionFile = baseFolderPath + file;
        }
    }
    if (streamDescriptionFile.empty()) {
        throw std::runtime_error("Missing stream description file");
    }

    std::ifstream inputFileStream(streamDescriptionFile, std::ios::in);
    if (!inputFileStream.is_open()) {
        throw std::runtime_error("DataLogger unable to open config file");
    }

    google::protobuf::io::IstreamInputStream inputStream(&inputFileStream);

    data_logger::DataLoggerConfig config;
    if (!google::protobuf::TextFormat::Parse(&inputStream, &config)) {
        throw std::runtime_error("DataLogger failed to parse config file");
    }

    for (int i = 0; i < config.camera_size(); i++) {
        const std::string& streamName = config.camera(i).name();
        struct ProtodatStats protodatStats = getProtodatStats<hal::CameraSample>(baseFolderPath + streamName + "/");
        LOG(INFO) << "Logged stats for camera: " << streamName;
        logProtodatStats(protodatStats);
    }

    for (int i = 0; i < config.gps_size(); i++) {
        const std::string& streamName = config.gps(i).name();
        struct ProtodatStats protodatStats = getProtodatStats<hal::GPSTelemetry>(baseFolderPath + streamName + "/");
        LOG(INFO) << "Logged stats for gps: " << streamName;
        logProtodatStats(protodatStats);
    }

    for (int i = 0; i < config.joystick_size(); i++) {
        const std::string& streamName = config.joystick(i).name();
        struct ProtodatStats protodatStats = getProtodatStats<hal::JoystickSample>(baseFolderPath + streamName + "/");
        LOG(INFO) << "Logged stats for joystick: " << streamName;
        logProtodatStats(protodatStats);
    }

    for (int i = 0; i < config.network_health_size(); i++) {
        const std::string& streamName = config.network_health(i).name();
        struct ProtodatStats protodatStats = getProtodatStats<hal::NetworkHealthTelemetry>(baseFolderPath + streamName + "/");
        LOG(INFO) << "Logged stats for Network Health stream: " << streamName;
        logProtodatStats(protodatStats);
    }

    for (int i = 0; i < config.vcu_telemetry_size(); i++) {
        const std::string& streamName = config.vcu_telemetry(i).name();
        struct ProtodatStats protodatStats = getProtodatStats<hal::VCUTelemetryEnvelope>(baseFolderPath + streamName + "/");
        LOG(INFO) << "Logged stats for VcuTelemetry Stream: " << streamName;
        logProtodatStats(protodatStats);
    }
}
