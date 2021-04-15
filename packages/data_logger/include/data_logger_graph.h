
#pragma once

#include "packages/data_logger/proto/config.pb.h"
#include "packages/filter_graph/include/thread_pool.h"

#include <memory>

namespace data_logger {

class DeviceSourceFilter;
class DeviceFileSinkFilter;
class SDLRenderSinkFilter;

class DataLoggerGraph {
public:
    DataLoggerGraph(const DataLoggerConfig& config, const std::string& dataOutputDir, const bool displayVideoStreams);
    ~DataLoggerGraph();
    DataLoggerGraph(const DataLoggerGraph&) = delete;
    DataLoggerGraph(const DataLoggerGraph&&) = delete;
    DataLoggerGraph& operator=(const DataLoggerGraph&) = delete;
    DataLoggerGraph& operator=(const DataLoggerGraph&&) = delete;

    void start();
    void stop();

private:
    std::shared_ptr<DeviceSourceFilter> m_deviceSourceFilter;
    std::shared_ptr<DeviceFileSinkFilter> m_deviceFileSinkFilter;
    std::shared_ptr<SDLRenderSinkFilter> m_sdlRenderSinkFilter;

    filter_graph::ThreadPool<filter_graph::SourceFilterThreadRunner> m_sourceFilterThreadPool;
    filter_graph::ThreadPool<filter_graph::FilterThreadRunner> m_filterThreadPool;

    void wireGraph();
};
}
