#include "renderer_api_glcore.h"
#include "glog/logging.h"
#include <cassert>
#include <iostream>

using namespace unity_plugins;

RendererAPI_GLCore::RendererAPI_GLCore() {}

RendererAPI_GLCore::~RendererAPI_GLCore() { ReleaseResources(); }

void RendererAPI_GLCore::ProcessDeviceEvent(UnityGfxDeviceEventType eventType, IUnityInterfaces* interfaces) {
    switch (eventType) {
    case kUnityGfxDeviceEventInitialize:
        CreateResources();
        break;
    case kUnityGfxDeviceEventShutdown:
        ReleaseResources();
        break;
    default:
        break;
    }
}

void RendererAPI_GLCore::Initialize_MainThread(int downloadQueueSize) {

    assert(downloadQueueSize > 0);
    m_downloadQueueSize = downloadQueueSize;
}

Status RendererAPI_GLCore::StartDownload_MainThread(const FrameData& frameData) {

    auto fd = std::make_shared<FrameData>(frameData);
    std::lock_guard<std::mutex> guard(m_pendingFramesMutex);
    m_pendingFrames.push(fd);

    return Status::Succeeded;
}

Status RendererAPI_GLCore::StartDownload_RenderThread() {

    std::lock_guard<std::mutex> guard(m_pendingFramesMutex);

    while (!m_pendingFrames.empty()) {
        auto frameData = m_pendingFrames.front();
        m_pendingFrames.pop();

        auto queue = GetQueue(frameData->cameraId, frameData->imageType);
        if (queue == nullptr) {
            queue = initializeQueue(frameData);
        }

        queue->StartDownload(frameData);
    }

    return Status::Succeeded;
}

std::shared_ptr<ImageDownloadQueue> RendererAPI_GLCore::initializeQueue(std::shared_ptr<FrameData> frameData) {
    // lazy initialization

    if (frameData->width < 2 || frameData->height < 2) {
        throw std::runtime_error("Texture sizes are too small. Must be 2x2 or larger");
    }

    auto group = m_cameraDownloads[frameData->cameraId];
    if (group == nullptr) {
        group = std::make_shared<CameraDownloadGroup>(m_downloadQueueSize);
        m_cameraDownloads[frameData->cameraId] = group;
    }

    // TODO actually extract the gl format and size data from opengl using pointer in framedata
    // and possible combine into GetQueue

    auto queue = group->SetQueue(frameData->imageType, frameData->width, frameData->height);
    queue->SetCallbacks(Callbacks());
    return queue;
}

Status RendererAPI_GLCore::UpdateDownloads_RenderThread() {

    Status status = Status::Succeeded;

    for (auto& kvp : m_cameraDownloads) {
        auto downloadStatus = kvp.second->UpdateDownloads();
        if (downloadStatus != Status::Succeeded) {
            status = downloadStatus;
        }
    }

    return status;
}

void RendererAPI_GLCore::CreateResources() {
    glewExperimental = GL_TRUE;
    glewInit();
    glGetError(); // clear glew error
}

void RendererAPI_GLCore::ReleaseResources() {

    std::lock_guard<std::mutex> guardP(m_pendingFramesMutex);
    while (!m_pendingFrames.empty()) {
        m_pendingFrames.pop();
    }

    m_cameraDownloads.clear();
}

std::shared_ptr<ImageDownloadQueue> RendererAPI_GLCore::GetQueue(hal::CameraId cameraId, ImageType imageType) {
    auto group = m_cameraDownloads[cameraId];
    if (group == nullptr) {
        return nullptr;
    }

    return group->Queue(imageType);
}

void RendererAPI_GLCore::CallbacksUpdated(const std::unordered_set<ProcessImageFuncPtr>& callbacks) {

    for (auto& kvp : m_cameraDownloads) {
        kvp.second->SetCallbacks(callbacks);
    }
}

void RendererAPI_GLCore::CleanUp_MainThread(hal::CameraId cameraId) {
    for (int ii = 0; ii < ImageType_ARRAYSIZE; ++ii) {
        CleanUp_MainThread(cameraId, (ImageType)ii);
    }
}

void RendererAPI_GLCore::CleanUp_MainThread(hal::CameraId cameraId, ImageType imageType) {
    std::lock_guard<std::mutex> guard(m_pendingCleanUpItemsMutex);
    auto item = std::make_shared<CleanUpItem>();
    item->cameraId = cameraId;
    item->imageType = imageType;

    m_pendingCleanUpItems.push(item);
}

void RendererAPI_GLCore::CleanUp_RenderThread() {
    std::lock_guard<std::mutex> guard(m_pendingCleanUpItemsMutex);

    while (!m_pendingCleanUpItems.empty()) {
        auto item = m_pendingCleanUpItems.front();
        m_pendingCleanUpItems.pop();

        if (m_cameraDownloads.count(item->cameraId) > 0) {
            m_cameraDownloads[item->cameraId]->RemoveQueue(item->imageType);
        }
    }
}

std::shared_ptr<RendererAPI> CreateRendererAPI_GLCore() { return std::make_shared<RendererAPI_GLCore>(); }
