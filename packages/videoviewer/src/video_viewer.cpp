
#include "packages/videoviewer/include/video_viewer.h"
#include "glog/logging.h"
#include <math.h>

using namespace video_viewer;

VideoViewer::VideoViewer(const std::string& serverAddress, const std::string& topic)
    : m_windowName(serverAddress)
    , m_context(1)
    , m_cameraSubscriber(m_context, serverAddress, topic, 1)
    , m_cancel(false)
    , m_thread(&VideoViewer::invoke, this)
    , m_previousTimestamp(0) {}

VideoViewer::~VideoViewer() {
    m_cancel = true;
    m_thread.join();
}

void VideoViewer::invoke() {
    while (!m_cancel) {
        if (m_cameraSubscriber.poll(std::chrono::milliseconds(1))) {
            hal::CameraSample sample;
            if (m_cameraSubscriber.recv(sample)) {
                if (!m_window.get()) {
                    m_window = std::make_shared<SDLWindow>(0, 0, sample.image().cols(), sample.image().rows(), m_windowName);
                }
                const uint64_t timestamp = sample.systemtimestamp().nanos();
                const double dt = ((double)timestamp - (double)m_previousTimestamp) / (double)1e9;
                const int fps = std::round(1.0 / dt);
                m_previousTimestamp = timestamp;
                m_window->setImage(sample, m_windowName + " fps:" + std::to_string(fps));
            } else {
                LOG(ERROR) << "Failed to recv image sample";
            }
        }
    }
}
