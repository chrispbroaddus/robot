
#pragma once

#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/videoviewer/include/sdl_window.h"
#include <thread>

namespace video_viewer {

///
/// VideoViewer connects to a ZMQ video publisher and creates an SDL window to visualize.
///
class VideoViewer {
public:
    VideoViewer() = delete;
    VideoViewer(const VideoViewer& other) = delete;
    VideoViewer(const std::string& serverAddress, const std::string& topic);
    ~VideoViewer();

private:
    /// Name give to the SDL window
    const std::string m_windowName;

    /// ZMQ context for thread management
    zmq::context_t m_context;

    /// ZMQ camera subscriber
    net::ZMQProtobufSubscriber<hal::CameraSample> m_cameraSubscriber;

    /// Set to true to cancel the thread and exit loop
    bool m_cancel;

    /// Thead that renders the video
    std::thread m_thread;

    /// SDL window to pop up
    std::shared_ptr<SDLWindow> m_window;

    /// Previous frametime to compute the FPS
    uint64_t m_previousTimestamp;

    /// Functioned invoked by the thread
    void invoke();
};
}
