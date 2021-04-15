
#include "packages/data_logger/include/sdl_render_sink_filter.h"
#include "packages/data_logger/include/data_logger_sample.h"
#include "packages/videoviewer/include/sdl_window.h"

#include "SDL.h"

#include <cmath>

using namespace video_viewer;

namespace data_logger {

SDLRenderSinkFilter::SDLRenderSinkFilter()
    : filter_graph::SinkFilter("SDLRenderSinkFilter", 10)
    , m_windowWidth(640)
    , m_windowsPerRow(4) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error("Unable to initialize SDL video");
    }
}

SDLRenderSinkFilter::~SDLRenderSinkFilter() { SDL_Quit(); }

void SDLRenderSinkFilter::receive(std::shared_ptr<filter_graph::Container> container) {

    for (const auto& cameraStreamId : m_cameraStreamIds) {

        auto sample = container->get(cameraStreamId);
        if (!sample.get()) {
            continue;
        }

        auto* cameraSample = static_cast<data_logger::details::DataloggerSample<hal::CameraSample>*>(sample.get());

        auto iterWindow = m_windows.find(cameraStreamId);
        if (iterWindow == m_windows.end()) {

            // Resize the window so that they can be distrubuted across the screen layout
            const float aspectRatio = (float)cameraSample->data().image().cols() / (float)cameraSample->data().image().rows();
            const size_t windowHeight = std::round(m_windowWidth * (1.f / aspectRatio));

            // Create the SDL window
            std::shared_ptr<SDLWindow> renderWindow = std::make_shared<SDLWindow>(0, 0, m_windowWidth, windowHeight, cameraStreamId);

            m_windows.insert(iterWindow, std::make_pair(cameraStreamId, renderWindow));

            positionWindows();
        }

        struct timespec currentFrameTime;
        clock_gettime(CLOCK_MONOTONIC, &currentFrameTime);

        auto iterTime = m_lastFrameTime.find(cameraStreamId);
        if (iterTime == m_lastFrameTime.end())
            m_lastFrameTime.insert(iterTime, std::make_pair(cameraStreamId, currentFrameTime));

        double fps1 = 1.0 / ((currentFrameTime.tv_sec - m_lastFrameTime[cameraStreamId].tv_sec)
                                + (currentFrameTime.tv_nsec - m_lastFrameTime[cameraStreamId].tv_nsec) / 1.0e9);
        double fps2 = 1.0e9 / (cameraSample->data().systemtimestamp().nanos() - m_lastFrameTimeUnity[cameraStreamId].nanos());
        const std::string title = cameraStreamId + " fps(SDL): " + std::to_string(fps1) + " fps(Unity): " + std::to_string(fps2);

        m_lastFrameTime[cameraStreamId] = currentFrameTime;
        m_lastFrameTimeUnity[cameraStreamId] = cameraSample->data().systemtimestamp();

        m_windows[cameraStreamId]->setImage(cameraSample->data(), title);
    }
}

std::tuple<size_t, size_t> SDLRenderSinkFilter::getWindowLayoutPosition(const size_t windowIndex) const {

    const size_t row = std::floor((float)windowIndex / (float)m_windowsPerRow);
    const size_t col = windowIndex % m_windowsPerRow;

    return std::make_tuple(row, col);
}

size_t SDLRenderSinkFilter::getMaxWindowHeight() const {

    size_t maxHeight = 0;
    for (const auto& window : m_windows) {
        if (window.second->height() > maxHeight) {
            maxHeight = window.second->height();
        }
    }

    return maxHeight;
}

void SDLRenderSinkFilter::positionWindows() {

    size_t windowIndex = 0;
    for (const auto& window : m_windows) {

        const std::tuple<size_t, size_t> windowLocation = getWindowLayoutPosition(windowIndex);
        const size_t windowPositionX = std::get<1>(windowLocation) * m_windowWidth;
        const size_t windowPositionY = std::get<0>(windowLocation) * getMaxWindowHeight();

        window.second->setWindowPosition(windowPositionX, windowPositionY);

        windowIndex++;
    }
}
}
