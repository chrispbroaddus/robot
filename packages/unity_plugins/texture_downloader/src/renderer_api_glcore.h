#pragma once

#include "camera_download_group.h"
#include "platform_base.h"
#include "renderer_api.h"
#include <GL/glew.h>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

#if SUPPORT_OPENGL_CORE

namespace unity_plugins {

///
/// @brief GL Core implementation of the RendererAPI
///
class RendererAPI_GLCore : public RendererAPI {
public:
    RendererAPI_GLCore();
    virtual ~RendererAPI_GLCore();

    void Initialize_MainThread(int downloadQueueSize);

    void ProcessDeviceEvent(UnityGfxDeviceEventType eventType, IUnityInterfaces* interfaces);

    Status StartDownload_MainThread(const FrameData& frameData);
    Status StartDownload_RenderThread();

    Status UpdateDownloads_RenderThread();

    void CleanUp_MainThread(hal::CameraId cameraId);
    void CleanUp_MainThread(hal::CameraId cameraId, ImageType imageType);

    void CleanUp_RenderThread();

protected:
    void CallbacksUpdated(const std::unordered_set<ProcessImageFuncPtr>& callbacks);

private:
    void CreateResources();
    void ReleaseResources();

    std::shared_ptr<ImageDownloadQueue> initializeQueue(std::shared_ptr<FrameData> frameData);
    std::shared_ptr<ImageDownloadQueue> GetQueue(hal::CameraId cameraId, ImageType imageType);

private:
    std::map<hal::CameraId, std::shared_ptr<CameraDownloadGroup> > m_cameraDownloads;

    int m_downloadQueueSize;

    std::queue<std::shared_ptr<FrameData> > m_pendingFrames;
    std::mutex m_pendingFramesMutex; /// Mutex to lock access to the pending frames

    struct CleanUpItem {
        hal::CameraId cameraId;
        ImageType imageType;
    };
    std::queue<std::shared_ptr<CleanUpItem> > m_pendingCleanUpItems;
    std::mutex m_pendingCleanUpItemsMutex;
};
} // namespace unity_plugins

///
/// @brief create a GL Core Renderer
///
std::shared_ptr<unity_plugins::RendererAPI> CreateRendererAPI_GLCore();

#endif // SUPPORT_OPEN_GL_CORE
