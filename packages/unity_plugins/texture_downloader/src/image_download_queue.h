#pragma once

#include "frame_data.h"
#include "packages/unity_plugins/utils/include/zippy_image_interop.h"
#include "status.h"
#include <GL/glew.h>
#include <atomic>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_set>
#include <vector>

namespace unity_plugins {

////
/// @brief ImageDownloadQueue contains a pool of PBOs and allocated memory to download images.
/// Images are downloaded asynchonously
///
class ImageDownloadQueue {
    struct DownloadItem;

public:
    ////
    /// @brief Create a download queue of the specified size and for the specifed image type
    ///
    ImageDownloadQueue(size_t queueSize, ImageType imageType);
    ~ImageDownloadQueue();

    ////
    /// @brief Actually create the queue for images of the specified size. Call again to recreate with a different size
    ///
    void Create(size_t width, size_t height);

    ////
    /// @brief Start a download
    ///
    Status StartDownload(std::shared_ptr<FrameData> frameData);

    ////
    /// @brief Update the downloads
    ///
    Status UpdateDownloads();

    void SetCallbacks(const std::unordered_set<ProcessImageFuncPtr>& processingFuncPtrs);

private:
    ////
    /// @brief Add a download item to the pool
    ///
    void AddDownloadItem(size_t dataSize);

    ////
    /// @brief Clean up a download item
    ///
    void RemoveDownloadItem(std::shared_ptr<DownloadItem> item);

    ////
    /// @brief Resize a download item
    ///
    void ResizeDownloadItem(std::shared_ptr<DownloadItem> downloadItem, size_t newDataSize);

    ////
    /// @brief Call the processing function with the downloaded texture
    ///
    struct DownloadItem;

    void EnqueueImage(std::shared_ptr<DownloadItem> downloadItem, void* data) __attribute__((nonnull));

    void SendRunner();

    void Unmap();
    void UnmapItem(std::shared_ptr<DownloadItem> downloadItem);

private:
    size_t m_queueSize;
    size_t m_dataSize;
    GLenum m_imageFormat;
    GLenum m_dataType;
    size_t m_bytesPerPixel;

    ////
    /// @brief Download item holds the PBO id, size of the data, pointers to the frameData and the sync object
    ///
    struct DownloadItem {
        GLuint pboId;
        GLuint tmpBufferId;
        size_t dataSize;
        std::shared_ptr<FrameData> frameData;
        std::shared_ptr<GLsync> sync;
    };

    struct SendItem {
        std::shared_ptr<DownloadItem> downloadItem;
        void* data;
    };

    std::unordered_set<std::shared_ptr<DownloadItem> > m_available;
    std::queue<std::shared_ptr<DownloadItem> > m_inUse;

    std::queue<std::shared_ptr<SendItem> > m_sendQueue;
    std::queue<std::shared_ptr<SendItem> > m_unmapQueue;
    std::thread m_executionThread;
    std::mutex m_sendMutex;
    std::mutex m_unmapMutex;

    std::atomic_bool m_runProcessor;

    std::unordered_set<ProcessImageFuncPtr> m_processingFuncPtrs;
    std::mutex m_processingFuncPtrsMutex;
};
} // namespace unity_plugins
