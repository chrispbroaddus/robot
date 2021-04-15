
#pragma once

#include "packages/data_logger/proto/config.pb.h"
#include "packages/filter_graph/include/sink_filter.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/image_codec/include/image_encoder_interface.h"
#include "packages/serialization/include/protobuf_io.h"

#include <fstream>

#include "glog/logging.h"

namespace data_logger {

class DeviceFileSinkFilter : public filter_graph::SinkFilter {
public:
    typedef std::map<std::string, std::pair<std::shared_ptr<std::ofstream>, std::shared_ptr<serialization::ProtobufWriter> > >
        file_stream_map_t;

    DeviceFileSinkFilter(const DataLoggerConfig& config, const std::string& dataOutputDir);
    ~DeviceFileSinkFilter();
    DeviceFileSinkFilter(const DeviceFileSinkFilter&) = delete;
    DeviceFileSinkFilter(const DeviceFileSinkFilter&&) = delete;
    DeviceFileSinkFilter& operator=(const DeviceFileSinkFilter&) = delete;
    DeviceFileSinkFilter& operator=(const DeviceFileSinkFilter&&) = delete;

    void receive(std::shared_ptr<filter_graph::Container> container) override;
    void setCameraStreamIds(const std::vector<std::string>& streamIds) { m_cameraStreamIds = streamIds; }
    void setJoystickStreamIds(const std::vector<std::string>& streamIds) { m_joystickStreamIds = streamIds; }
    void setVcuTelemetryStreamIds(const std::vector<std::string>& streamIds) { m_vcuTelemetryStreamIds = streamIds; }
    void setGpsStreamIds(const std::vector<std::string>& streamIds) { m_gpsStreamIds = streamIds; }
    void setImuStreamIds(const std::vector<std::string>& streamIds) { m_imuStreamIds = streamIds; }
    void setNetworkHealthStreamIds(const std::vector<std::string>& streamIds) { m_networkHealthStreamIds = streamIds; }

private:
    const std::string m_dataOutputDir;
    std::vector<std::string> m_cameraStreamIds;
    std::vector<std::string> m_joystickStreamIds;
    std::vector<std::string> m_vcuTelemetryStreamIds;
    std::vector<std::string> m_gpsStreamIds;
    std::vector<std::string> m_imuStreamIds;
    std::vector<std::string> m_networkHealthStreamIds;
    std::unique_ptr<image_codec::ImageEncoder> m_encoder;

    const size_t m_maxFileSizeInBytes;
    file_stream_map_t m_outputStreams;

    void stop();
    void writeToFile(const std::string& streamId, const google::protobuf::MessageLite& msg);
};
}
