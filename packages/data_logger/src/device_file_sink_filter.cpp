#include "packages/data_logger/include/device_file_sink_filter.h"
#include "packages/data_logger/include/data_logger_sample.h"
#include "packages/hal/proto/gps_telemetry.pb.h"
#include "packages/hal/proto/imu_sample.pb.h"
#include "packages/hal/proto/joystick_sample.pb.h"
#include "packages/hal/proto/network_health_telemetry.pb.h"
#include "packages/hal/proto/vcu_telemetry_envelope.pb.h"
#include "packages/image_codec/include/jpeg/jpeg_encoder.h"
#include "packages/image_codec/include/passthrough/passthrough_encoder.h"
#include "packages/image_codec/include/png/png_encoder.h"
#include "packages/serialization/include/protobuf_io.h"

#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <ctime>
#include <stdlib.h>

using namespace filter_graph;

static std::string getPrettyDataFileString() {
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[256];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", timeinfo);
    return std::string(buffer);
}

namespace data_logger {

static int makeDirectory(const std::string& dir) {
    const std::string cmd = "mkdir -p " + dir;
    return system(cmd.c_str());
}

/// Protobuf has a limit on the maximum number of bytes that a CodedInputStream will read before refusing to continue
/// The theoretical shortest message that could cause integer overflows is 512MB
/// Setting the MAX_FILE_SIZE to the warning threshold we set
constexpr size_t MAX_FILE_SIZE = serialization::WARNING_THRESHOLD;

DeviceFileSinkFilter::DeviceFileSinkFilter(const DataLoggerConfig& config, const std::string& dataOutputDir)
    : filter_graph::SinkFilter("DeviceFileSinkFilter", 10)
    , m_dataOutputDir(dataOutputDir)
    , m_maxFileSizeInBytes(MAX_FILE_SIZE) {

    LOG(INFO) << "Creating directory: " << dataOutputDir;
    makeDirectory(dataOutputDir);

    if (config.has_jpeg_encoder_options()) {
        m_encoder.reset(new image_codec::JpegEncoder(config.jpeg_encoder_options().quality()));
    } else if (config.has_png_encoder_options()) {
        m_encoder.reset(new image_codec::PngEncoder(config.png_encoder_options().compression_level()));
    } else {
        LOG(INFO) << "DataLogger: No encoder options found. Using default passthrough encoder";
        m_encoder.reset(new image_codec::PassthroughEncoder());
    }
}

DeviceFileSinkFilter::~DeviceFileSinkFilter() { stop(); }

void DeviceFileSinkFilter::receive(std::shared_ptr<filter_graph::Container> container) {

    for (const auto& cameraStreamId : m_cameraStreamIds) {
        auto sample = container->get(cameraStreamId);
        if (!sample.get()) {
            continue;
        }
        auto cameraSample = static_cast<data_logger::details::DataloggerSample<hal::CameraSample>*>(sample.get());
        hal::CameraSample cameraImage = cameraSample->data();
        if (cameraImage.image().format() == hal::PB_LUMINANCE || cameraImage.image().format() == hal::PB_RAW
            || cameraImage.image().format() == hal::PB_RGB || cameraImage.image().format() == hal::PB_BGR) {
            m_encoder->encode(cameraImage.image(), *(cameraImage.mutable_image()));
        } else {
            LOG_EVERY_N(WARNING, 500) << "Image of type: " << cameraImage.image().format() << " not being encoded";
        }
        writeToFile(cameraStreamId, cameraImage);
    }

    for (const auto& joystickStreamIds : m_joystickStreamIds) {
        auto sample = container->get(joystickStreamIds);
        if (!sample.get()) {
            continue;
        }
        auto joystickSample = static_cast<data_logger::details::DataloggerSample<hal::JoystickSample>*>(sample.get());
        writeToFile(joystickStreamIds, joystickSample->data());
    }

    for (const auto& telemetryStreamIds : m_vcuTelemetryStreamIds) {
        auto sample = container->get(telemetryStreamIds);
        if (!sample.get()) {
            continue;
        }
        auto telemetrySample = static_cast<data_logger::details::DataloggerSample<hal::VCUTelemetryEnvelope>*>(sample.get());
        writeToFile(telemetryStreamIds, telemetrySample->data());
    }

    for (const auto& gpsStreamIds : m_gpsStreamIds) {
        auto sample = container->get(gpsStreamIds);
        if (!sample.get()) {
            continue;
        }
        auto gpsSample = static_cast<data_logger::details::DataloggerSample<hal::GPSTelemetry>*>(sample.get());
        writeToFile(gpsStreamIds, gpsSample->data());
    }

    for (const auto& imuStreamIds : m_imuStreamIds) {
        auto sample = container->get(imuStreamIds);
        if (!sample.get()) {
            continue;
        }
        auto imuSample = static_cast<data_logger::details::DataloggerSample<hal::IMUSample>*>(sample.get());
        writeToFile(imuStreamIds, imuSample->data());
    }

    for (const auto& networkHealthStreamIds : m_networkHealthStreamIds) {
        auto sample = container->get(networkHealthStreamIds);
        if (!sample.get()) {
            continue;
        }
        auto networkHealthSample = static_cast<data_logger::details::DataloggerSample<hal::NetworkHealthTelemetry>*>(sample.get());
        writeToFile(networkHealthStreamIds, networkHealthSample->data());
    }
}

void DeviceFileSinkFilter::stop() { m_outputStreams.clear(); }

void DeviceFileSinkFilter::writeToFile(const std::string& streamId, const google::protobuf::MessageLite& msg) {

    // If a file already exists for the stream, then check if the size is too large. If so, then close the file
    // and remove the file from m_outputStreams map.
    file_stream_map_t::iterator filestreamIterator = m_outputStreams.find(streamId);
    if (filestreamIterator != m_outputStreams.end()) {
        if (filestreamIterator->second.second->size() + msg.ByteSize() > m_maxFileSizeInBytes) {
            LOG(INFO) << "Closing file for stream: " << filestreamIterator->first;
            filestreamIterator->second.first->close();
            m_outputStreams.erase(filestreamIterator);
        }
    }

    // If there isn't a file created for the stream, then create a stream and store it.
    if (m_outputStreams.find(streamId) == m_outputStreams.end()) {

        const std::string streamDir = m_dataOutputDir + "/" + streamId;
        makeDirectory(streamDir);

        const std::string filename = streamDir + "/" + streamId + "-" + getPrettyDataFileString() + ".protodat";

        LOG(INFO) << "Creating file for stream: " << streamId << ": " << filename;

        auto outputFileStream = std::make_shared<std::ofstream>(filename, std::ios::out | std::ios::binary);
        if (!outputFileStream->is_open()) {
            throw std::runtime_error("Unable to open file: " + filename);
        }

        auto protobufWriter = std::make_shared<serialization::ProtobufWriter>(outputFileStream.get());

        m_outputStreams[streamId].first = outputFileStream;
        m_outputStreams[streamId].second = protobufWriter;
    }

    if (!m_outputStreams[streamId].second->writeNext(msg)) {
        throw std::runtime_error("Unable to write to proto file");
    }
}
}