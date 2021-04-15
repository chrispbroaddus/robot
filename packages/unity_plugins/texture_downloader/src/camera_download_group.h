#pragma once

#include "image_download_queue.h"
#include "packages/unity_plugins/utils/include/zippy_image_interop.h"
#include <map>
#include <memory>

namespace unity_plugins {

////
/// @brief Holds the download queues for a particular camera
///
class CameraDownloadGroup {
public:
    ////
    /// @brief Create a CameraDownloadGroup with a specified queue size
    ///
    CameraDownloadGroup(int queueSize);

    ////
    /// @brief Create or recreate a queue for an image type with a specified size
    ///
    std::shared_ptr<ImageDownloadQueue> SetQueue(ImageType imageType, int width, int height);

    ////
    /// @brief Remove the queue for the specified image type
    ///
    void RemoveQueue(ImageType imageType);

    ////
    /// @brief Get the specified queue. Null if not available
    ///
    std::shared_ptr<ImageDownloadQueue> Queue(ImageType imageType);

    ////
    /// @brief Update the downloads
    ///
    Status UpdateDownloads();

    ////
    /// @brief Set the callbacks that will be called when a download completes
    ///
    void SetCallbacks(const std::unordered_set<ProcessImageFuncPtr>& processingFuncPtrs);

private:
    int m_queueSize;
    std::map<ImageType, std::shared_ptr<ImageDownloadQueue> > m_downloadQueues;
};
} // namespace unity_plugins
