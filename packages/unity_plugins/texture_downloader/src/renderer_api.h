#pragma once

#include "Unity/IUnityGraphics.h"
#include "frame_data.h"
#include "packages/core/include/wait_queue.h"
#include "packages/unity_plugins/utils/include/zippy_image_interop.h"
#include "status.h"
#include <memory>
#include <mutex>
#include <unordered_set>

namespace unity_plugins {

///
/// @brief Base RenderAPI class. Provides a generic way to manage multiple APIs
///
class RendererAPI {
public:
    RendererAPI(){};
    virtual ~RendererAPI() = default;

    /// @brief First call initialize on the main thread to set variables
    virtual void Initialize_MainThread(int downloadQueueSize) = 0;

    /// @brief process an event from the graphics device
    virtual void ProcessDeviceEvent(UnityGfxDeviceEventType eventType, IUnityInterfaces* interfaces) = 0;

    /// @brief Start the download on the main thread. This should queue up the data
    virtual Status StartDownload_MainThread(const FrameData& frameData) = 0;

    /// @brief Start the download on the render thread. This should actually begin the download
    virtual Status StartDownload_RenderThread() = 0;

    /// @brief Update the downloads on the render thread
    virtual Status UpdateDownloads_RenderThread() = 0;

    /// @brief cleanup the downloader for the specified camera. ie free up the memory. This should queue up the request
    virtual void CleanUp_MainThread(hal::CameraId cameraId) = 0;
    /// @brief cleanup the downloader for the specified camera and image type. This should queue up the request
    virtual void CleanUp_MainThread(hal::CameraId cameraId, ImageType imageType) = 0;

    /// @brief run the clean up on the render thread
    virtual void CleanUp_RenderThread() = 0;

    /// @brief Add callback to get data out of plugin
    void AddProcessingCallback(ProcessImageFuncPtr processingFunc);

    /// @brief Remove callback to get data out of plugin
    void RemoveProcessingCallback(ProcessImageFuncPtr processingFunc);

protected:
    virtual void CallbacksUpdated(const std::unordered_set<ProcessImageFuncPtr>& callbacks) {}
    std::unordered_set<ProcessImageFuncPtr> Callbacks();

private:
    std::unordered_set<ProcessImageFuncPtr> m_processingFuncPtrs;
    std::mutex m_processingFuncPtrsMutex;
};
} // namespace unity_plugins

/// @brief Create the renderer api for the specified device
std::shared_ptr<unity_plugins::RendererAPI> CreateRendererAPI(UnityGfxRenderer apiType);
