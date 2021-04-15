#include "camera_download_group.h"

using namespace unity_plugins;

CameraDownloadGroup::CameraDownloadGroup(int queueSize)
    : m_queueSize(queueSize) {}

std::shared_ptr<ImageDownloadQueue> CameraDownloadGroup::SetQueue(ImageType imageType, int width, int height) {
    auto queue = m_downloadQueues[imageType];
    if (queue == nullptr) {
        queue = std::make_shared<ImageDownloadQueue>(m_queueSize, imageType);
        m_downloadQueues[imageType] = queue;
    }

    queue->Create(width, height);

    return queue;
}

void CameraDownloadGroup::RemoveQueue(ImageType imageType) {
    if (m_downloadQueues.count(imageType) > 0) {
        m_downloadQueues.erase(imageType);
    }
}

std::shared_ptr<ImageDownloadQueue> CameraDownloadGroup::Queue(ImageType imageType) { return m_downloadQueues[imageType]; }

Status CameraDownloadGroup::UpdateDownloads() {
    Status status = Status::Succeeded;
    for (auto& kvp : m_downloadQueues) {
        Status updateStatus = kvp.second->UpdateDownloads();
        if (updateStatus != Status::Succeeded) {
            status = updateStatus;
        }
    }

    return status;
}

void CameraDownloadGroup::SetCallbacks(const std::unordered_set<ProcessImageFuncPtr>& processingFuncPtrs) {
    for (auto& kvp : m_downloadQueues) {
        kvp.second->SetCallbacks(processingFuncPtrs);
    }
}