
#include "packages/data_logger/include/data_logger_graph.h"
#include "packages/data_logger/include/device_file_sink_filter.h"
#include "packages/data_logger/include/device_source_filter.h"
#include "packages/data_logger/include/sdl_render_sink_filter.h"

namespace data_logger {

DataLoggerGraph::DataLoggerGraph(const DataLoggerConfig& config, const std::string& dataOutputDir, const bool displayVideoStreams)
    : m_sourceFilterThreadPool(1)
    , m_filterThreadPool(2) {

    m_deviceSourceFilter = std::make_shared<DeviceSourceFilter>(config);
    m_deviceFileSinkFilter = std::make_shared<DeviceFileSinkFilter>(config, dataOutputDir);

    if (displayVideoStreams) {
        m_sdlRenderSinkFilter = std::make_shared<SDLRenderSinkFilter>();
    }

    wireGraph();
}

DataLoggerGraph::~DataLoggerGraph() {}

void DataLoggerGraph::start() {

    m_sourceFilterThreadPool.start();
    m_filterThreadPool.start();
}

void DataLoggerGraph::stop() {

    m_deviceSourceFilter->stop();
    m_sourceFilterThreadPool.stop();
    m_filterThreadPool.stop();
}

void DataLoggerGraph::wireGraph() {

    m_deviceSourceFilter->attachDownstreamFilter(m_deviceFileSinkFilter);

    m_deviceFileSinkFilter->setCameraStreamIds(m_deviceSourceFilter->cameraStreamIds());
    m_deviceFileSinkFilter->setGpsStreamIds(m_deviceSourceFilter->gpsStreamIds());
    m_deviceFileSinkFilter->setImuStreamIds(m_deviceSourceFilter->imuStreamIds());
    m_deviceFileSinkFilter->setJoystickStreamIds(m_deviceSourceFilter->joystickStreamIds());
    m_deviceFileSinkFilter->setNetworkHealthStreamIds(m_deviceSourceFilter->networkHealthStreamIds());
    m_deviceFileSinkFilter->setVcuTelemetryStreamIds(m_deviceSourceFilter->vcuTelemetryStreamIds());

    m_sourceFilterThreadPool.attachedFilter(m_deviceSourceFilter);
    m_filterThreadPool.attachedFilter(m_deviceFileSinkFilter);

    if (m_sdlRenderSinkFilter) {
        m_deviceSourceFilter->attachDownstreamFilter(m_sdlRenderSinkFilter);
        m_sdlRenderSinkFilter->setCameraStreamIds(m_deviceSourceFilter->cameraStreamIds());
        m_filterThreadPool.attachedFilter(m_sdlRenderSinkFilter);
    }

    m_sourceFilterThreadPool.initialize();
    m_filterThreadPool.initialize();
}
}
