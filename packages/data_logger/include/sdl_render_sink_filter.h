
#pragma once

#include "packages/core/proto/timestamp.pb.h"
#include "packages/filter_graph/include/sink_filter.h"
#include "packages/videoviewer/include/sdl_window.h"
#include <map>
#include <time.h>

namespace data_logger {

class SDLRenderSinkFilter : public filter_graph::SinkFilter {
public:
    SDLRenderSinkFilter();
    ~SDLRenderSinkFilter();

    void receive(std::shared_ptr<filter_graph::Container> container) override;
    void setCameraStreamIds(const std::vector<std::string>& streamIds) { m_cameraStreamIds = streamIds; }

private:
    std::vector<std::string> m_cameraStreamIds;
    std::map<std::string, std::shared_ptr<video_viewer::SDLWindow> > m_windows;
    std::map<std::string, struct timespec> m_lastFrameTime;
    std::map<std::string, core::SystemTimestamp> m_lastFrameTimeUnity;
    const size_t m_windowWidth;
    const size_t m_windowsPerRow;

    std::tuple<size_t, size_t> getWindowLayoutPosition(const size_t windowIndex) const;
    size_t getMaxWindowHeight() const;
    void positionWindows();
};
}
