#include "Unity/IUnityGraphics.h"
#include "platform_base.h"
#include "renderer_api.h"
#include <cassert>
#include <memory>
#include <string>

using namespace unity_plugins;

static IUnityInterfaces* sUnityInterfaces;
static IUnityGraphics* sUnityGraphics;
static UnityGfxRenderer sDeviceType = kUnityGfxRendererNull;
static std::shared_ptr<RendererAPI> sCurrentAPI = nullptr;
static Status sLastStatus = Status::Succeeded;

///
/// @brief Run the Render thread start download function
///
static void ZIPPY_INTERFACE_API TextureDownloader_OnStartDownloadEvent(int) {
    if (sCurrentAPI) {
        sLastStatus = sCurrentAPI->StartDownload_RenderThread();
    }
}

///
/// @brief Run the render thread update downloads function
///
static void ZIPPY_INTERFACE_API TextureDownloader_OnUpdateDownloadsEvent(int) {
    if (sCurrentAPI) {
        sLastStatus = sCurrentAPI->UpdateDownloads_RenderThread();
    }
}

///
/// @brief Cleanup a download queue
///
static void ZIPPY_INTERFACE_API TextureDownloader_OnCleanUpEvent(int) {
    if (sCurrentAPI) {
        sCurrentAPI->CleanUp_RenderThread();
    }
}

///
/// @brief Initialize the downloader
///
extern "C" void ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API TextureDownloader_Initialize(int downloadQueueSize) {
    if (sCurrentAPI) {
        sCurrentAPI->Initialize_MainThread(downloadQueueSize);
    }
}

///
/// @brief Start downloading the specified texture
///
extern "C" int ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API TextureDownloader_StartDownload(FrameData frameData) {
    if (frameData.texturePtr == NULL) {
        sLastStatus = Status::Error_InvalidArguments;
    } else if (sCurrentAPI) {
        sLastStatus = sCurrentAPI->StartDownload_MainThread(frameData);
    } else {
        sLastStatus = Status::Error_UnsupportedAPI;
    }

    return (int)sLastStatus;
}

///
/// @brief Clean up the specified download queue
///
extern "C" void ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API TextureDownloader_CleanUp(hal::CameraId cameraId, ImageType imageType) {
    if (sCurrentAPI) {
        sCurrentAPI->CleanUp_MainThread(cameraId, imageType);
    }
}

///
/// @brief Clean up all download queues for the specified camera0
///
extern "C" void ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API TextureDownloader_CleanUpAll(hal::CameraId cameraId) {
    if (sCurrentAPI) {
        sCurrentAPI->CleanUp_MainThread(cameraId);
    }
}

///
/// @brief Get the function pointer to the render thread start downloading function
///
extern "C" UnityRenderingEvent ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API TextureDownloader_GetStartDownloadFunctionPtr() {
    return TextureDownloader_OnStartDownloadEvent;
}

///
/// @brief Get the function pointer to the render thread update downloads function
///
extern "C" UnityRenderingEvent ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API TextureDownloader_GetUpdateDownloadsFunctionPtr() {
    return TextureDownloader_OnUpdateDownloadsEvent;
}

///
/// @brief Get the function pointer to the render thread cleanup function
///
extern "C" UnityRenderingEvent ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API TextureDownloader_GetCleanUpFunctionPtr() {
    return TextureDownloader_OnCleanUpEvent;
}

///
/// @brief Handle an event from the graphics device
///
static void ZIPPY_INTERFACE_API TextureDownloader_OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType) {
    if (eventType == kUnityGfxDeviceEventInitialize) {
        assert(sCurrentAPI == NULL);
        sDeviceType = sUnityGraphics->GetRenderer();
        sCurrentAPI = CreateRendererAPI(sDeviceType);
    }

    if (sCurrentAPI != NULL) {
        sCurrentAPI->ProcessDeviceEvent(eventType, sUnityInterfaces);
    }

    if (eventType == kUnityGfxDeviceEventShutdown) {
        sCurrentAPI = nullptr;
        sDeviceType = kUnityGfxRendererNull;
    }
}

///
/// @brief Unity calls this to load the plugin
///
extern "C" void ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces) {
    sUnityInterfaces = unityInterfaces;
    sUnityGraphics = sUnityInterfaces->Get<IUnityGraphics>();
    sUnityGraphics->RegisterDeviceEventCallback(TextureDownloader_OnGraphicsDeviceEvent);

    TextureDownloader_OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
}

///
/// @brief Unity calls this to unload the plugin
///
extern "C" void ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API UnityPluginUnload() {
    sUnityGraphics->UnregisterDeviceEventCallback(TextureDownloader_OnGraphicsDeviceEvent);
}

///
/// @brief Get the last status
///
extern "C" int ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API TextureDownloader_GetLastStatus() { return (int)sLastStatus; }

///
/// @brief Set the callback to process the downloaded image. This should return ASAP, and must not keep a reference to the data pointer
///
extern "C" void ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API TextureDownloader_AddImageProcessingCallback(
    ProcessImageFuncPtr processingImageCallback) {
    if (sCurrentAPI) {
        sCurrentAPI->AddProcessingCallback(processingImageCallback);
    }
}

///
/// @brief Remove the callback to process the downloaded image
///
extern "C" void ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API TextureDownloader_RemoveImageProcessingCallback(
    ProcessImageFuncPtr processingImageCallback) {
    if (sCurrentAPI) {
        sCurrentAPI->RemoveProcessingCallback(processingImageCallback);
    }
}
