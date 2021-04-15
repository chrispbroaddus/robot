#include "packages/filesystem/include/filesystem.h"
#include "packages/serialization/include/protobuf_io.h"

#include "packages/data_logger/proto/config.pb.h"
#include "packages/hal/proto/camera_sample.pb.h"

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "src/google/protobuf/text_format.h"

#include <algorithm>
#include <fstream>

DEFINE_string(inputDir, "", "path to the directory containing the logged data");
DEFINE_string(outputDir, "", "path to the output directory");

static int makeDirectory(const std::string& dir) {
    const std::string cmd = "mkdir -p " + dir;
    return system(cmd.c_str());
}

/// Saves a single cameraSample as an image while in the specified output folder with the timestamp as the file name
void saveAsFile(const hal::CameraSample& cameraSample, std::string outputFolderPath) {

    makeDirectory(outputFolderPath);

    std::string filePath;
    if (cameraSample.image().format() == hal::PB_COMPRESSED_JPEG) {
        filePath = outputFolderPath + std::to_string(cameraSample.systemtimestamp().nanos()) + ".jpg";
    } else {
        throw std::runtime_error("Unsupported image format");
    }

    std::ofstream file(filePath);
    file << cameraSample.image().data();
    file.close();
}

/// Read images from the protodat files and save them as invidual files
void saveAsFiles(std::string protodatFolderPath, std::string outputFolderPath) {

    std::unique_ptr<serialization::ProtobufReader> protobufReader;
    hal::CameraSample cameraSample;
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
            if (protobufReader->readNext(cameraSample)) {
                saveAsFile(cameraSample, outputFolderPath);
            } else {
                break;
            }
        }
        inputFileStream.close();
    }
}

/// Utility to parse images stored by the dataLogger(in protodat format) and save them as individual image files with
/// the images from each camera stream in its own folder
int main(int argc, char* argv[]) {

    gflags::SetUsageMessage("Get statistics from the data logged by the datalogger");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    if (FLAGS_inputDir.empty() || FLAGS_outputDir.empty()) {
        gflags::ShowUsageWithFlagsRestrict(argv[0], "");
        return 0;
    }

    std::vector<std::string> fileList = filesystem::getFileList(FLAGS_inputDir);

    std::string baseFolderPath;
    baseFolderPath = FLAGS_inputDir.substr(FLAGS_inputDir.size() - 1) == "/" ? FLAGS_inputDir : FLAGS_inputDir + "/";
    std::string outputFolderPath;
    outputFolderPath = FLAGS_outputDir.substr(FLAGS_outputDir.size() - 1) == "/" ? FLAGS_outputDir : FLAGS_outputDir + "/";

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
        LOG(INFO) << "Saving images from camera: " << streamName;
        saveAsFiles(baseFolderPath + streamName + "/", outputFolderPath + streamName + "/");
        LOG(INFO) << "Saved images from camera: " << streamName;
    }
}