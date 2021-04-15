#include "image_download_queue.h"
#include <stdexcept>

#include "glog/logging.h"

using namespace unity_plugins;

ImageDownloadQueue::ImageDownloadQueue(size_t queueSize, ImageType imageType)
    : m_queueSize(queueSize)
    , m_dataSize(0)
    , m_runProcessor(true) {

    // This is what we want the downloaded data to be for each type. on the GPU is may be different
    switch (imageType) {
    case ImageType::Greyscale:
        m_imageFormat = GL_RED;
        m_dataType = GL_UNSIGNED_BYTE;
        m_bytesPerPixel = 1;
        break;
    case ImageType::Color:
        m_imageFormat = GL_RGB;
        m_dataType = GL_UNSIGNED_BYTE;
        m_bytesPerPixel = 3;
        break;
    case ImageType::Depth:
        m_imageFormat = GL_RED;
        m_dataType = GL_FLOAT;
        m_bytesPerPixel = 4;
        break;
    case ImageType::Pointcloud:
        m_imageFormat = GL_RGB;
        m_dataType = GL_FLOAT;
        m_bytesPerPixel = 12;
        break;
    default:
        throw std::invalid_argument("Unsupported image type");
    }

    m_executionThread = std::thread(&ImageDownloadQueue::SendRunner, this);
}

ImageDownloadQueue::~ImageDownloadQueue() {

    m_runProcessor = false;
    if (m_executionThread.joinable()) {
        m_executionThread.join();
    }

    while (!m_sendQueue.empty()) {
        auto item = m_sendQueue.front();
        m_sendQueue.pop();
        RemoveDownloadItem(item->downloadItem);
    }

    while (!m_unmapQueue.empty()) {
        auto item = m_unmapQueue.front();
        m_unmapQueue.pop();
        RemoveDownloadItem(item->downloadItem);
    }

    while (!m_inUse.empty()) {
        auto item = m_inUse.front();
        m_inUse.pop();
        RemoveDownloadItem(item);
    }

    while (!m_available.empty()) {
        auto item = *m_available.begin();
        m_available.erase(m_available.begin());

        RemoveDownloadItem(item);
    }
}

void ImageDownloadQueue::AddDownloadItem(size_t dataSize) {
    auto item = std::make_shared<DownloadItem>();

    ResizeDownloadItem(item, dataSize);

    m_available.insert(item);
}

void ImageDownloadQueue::RemoveDownloadItem(std::shared_ptr<DownloadItem> item) {
    glDeleteBuffers(1, &item->pboId);
    glDeleteBuffers(1, &item->tmpBufferId);

    if (item->sync) {
        glDeleteSync(*item->sync);
    }
}

void ImageDownloadQueue::ResizeDownloadItem(std::shared_ptr<DownloadItem> downloadItem, size_t newDataSize) {

    if (newDataSize == downloadItem->dataSize) {
        return;
    }

    if (downloadItem->pboId > 0) {
        glDeleteBuffers(1, &downloadItem->pboId);
    }
    if (downloadItem->tmpBufferId) {
        glDeleteBuffers(1, &downloadItem->tmpBufferId);
    }

    glCreateBuffers(1, &downloadItem->pboId);
    glNamedBufferStorage(downloadItem->pboId, newDataSize, nullptr, GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

    glCreateBuffers(1, &downloadItem->tmpBufferId);
    glNamedBufferStorage(downloadItem->tmpBufferId, newDataSize, nullptr, 0);

    downloadItem->dataSize = newDataSize;
}

void ImageDownloadQueue::Create(size_t newWidth, size_t newHeight) {
    size_t dataSize = newWidth * newHeight * m_bytesPerPixel;

    // if no change, do nothing
    if (dataSize == m_dataSize) {
        return;
    }

    m_dataSize = dataSize;

    while ((m_inUse.size() + m_available.size()) < m_queueSize) {
        AddDownloadItem(m_dataSize);
    }

    for (auto& downloadItem : m_available) {
        ResizeDownloadItem(downloadItem, m_dataSize);
    }
}

Status ImageDownloadQueue::StartDownload(std::shared_ptr<FrameData> frameData) {
    if (m_available.empty()) {
        LOG(ERROR) << "Too many requests";
        return Status::Error_TooManyRequests;
    }

    auto downloadItem = *m_available.begin();
    m_available.erase(m_available.begin());

    downloadItem->frameData = frameData;

    // updated using info from https://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf
    // using a temp buffer is faster

    // copy pixels from texture to temp PBO
    // Use offset instead of pointer.
    // OpenGL should perform async DMA transfer, so glGetTexImage() will return immediately.
    glBindBuffer(GL_PIXEL_PACK_BUFFER, downloadItem->tmpBufferId);
    glBindTexture(GL_TEXTURE_2D, (GLuint)(size_t)(frameData->texturePtr));
    glGetTexImage(GL_TEXTURE_2D, 0, m_imageFormat, m_dataType, 0);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    // copy from tmp buffer to output buffer
    glBindBuffer(GL_COPY_READ_BUFFER, downloadItem->tmpBufferId);
    glBindBuffer(GL_COPY_WRITE_BUFFER, downloadItem->pboId);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, downloadItem->dataSize);

    // set fence sync so we will know when the download is complete
    downloadItem->sync = std::make_shared<GLsync>(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));

    m_inUse.push(downloadItem);

    return Status::Succeeded;
}

Status ImageDownloadQueue::UpdateDownloads() {

    Status status = Status::Succeeded;

    while (!m_inUse.empty()) {
        // get the first item
        auto downloadItem = m_inUse.front();
        // check sync
        auto syncStatus = glClientWaitSync(*downloadItem->sync, 0, 0);

        if (syncStatus == GL_ALREADY_SIGNALED || syncStatus == GL_CONDITION_SATISFIED) {
            // remove the item from the queue
            m_inUse.pop();
            // map the PBO that contains pixels before processing it. This does the copy from gpu to cpu.
            void* pixels = glMapNamedBufferRange(
                downloadItem->pboId, 0, downloadItem->dataSize, GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

            if (pixels) {
                EnqueueImage(downloadItem, pixels);
            } else {
                status = Status::Error_DownloadNoData;
                UnmapItem(downloadItem);
            }
        } else if (syncStatus == GL_WAIT_FAILED) {
            // download failed, so abort and return to pool
            status = Status::Error_DownloadWait;
            m_inUse.pop();
            UnmapItem(downloadItem);
        } else {
            // download has not finished, so stop checking others
            break;
        }
    }

    Unmap();
    return status;
}

void ImageDownloadQueue::Unmap() {
    std::lock_guard<std::mutex> guard(m_unmapMutex);

    while (!m_unmapQueue.empty()) {
        auto item = m_unmapQueue.front();
        m_unmapQueue.pop();

        UnmapItem(item->downloadItem);
    }
}

void ImageDownloadQueue::UnmapItem(std::shared_ptr<DownloadItem> downloadItem) {

    glUnmapNamedBuffer(downloadItem->pboId);

    downloadItem->frameData = nullptr;

    // clean up the sync as can only use once
    glDeleteSync(*downloadItem->sync);
    downloadItem->sync = nullptr;

    if (downloadItem->dataSize != m_dataSize) {
        ResizeDownloadItem(downloadItem, m_dataSize);
    }

    m_available.insert(downloadItem);
}

void ImageDownloadQueue::SendRunner() {

    while (m_runProcessor) {
        if (m_sendQueue.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        m_sendMutex.lock();

        std::shared_ptr<SendItem> sendItem = m_sendQueue.front();
        m_sendQueue.pop();

        m_sendMutex.unlock();

        auto frameData = sendItem->downloadItem->frameData;

        CameraImage cameraImage;
        cameraImage.cameraId = frameData->cameraId;
        cameraImage.imageType = frameData->imageType;
        cameraImage.data = sendItem->data;
        cameraImage.width = frameData->width;
        cameraImage.height = frameData->height;
        cameraImage.bytesPerPixel = m_bytesPerPixel;
        cameraImage.frameNumber = frameData->frameNumber;
        cameraImage.timestamp = frameData->timestamp;

        std::lock_guard<std::mutex> guard(m_processingFuncPtrsMutex);
        for (auto& processingFuncPtr : m_processingFuncPtrs) {
            processingFuncPtr(cameraImage);
        }

        m_unmapMutex.lock();
        m_unmapQueue.push(sendItem);
        m_unmapMutex.unlock();
    }
}

void ImageDownloadQueue::EnqueueImage(std::shared_ptr<DownloadItem> downloadItem, void* data) {

    auto sendItem = std::make_shared<SendItem>();
    sendItem->downloadItem = downloadItem;
    sendItem->data = data;

    m_sendMutex.lock();
    m_sendQueue.push(sendItem);
    m_sendMutex.unlock();
}

void ImageDownloadQueue::SetCallbacks(const std::unordered_set<ProcessImageFuncPtr>& processingFuncPtrs) {

    std::lock_guard<std::mutex> guard(m_processingFuncPtrsMutex);
    m_processingFuncPtrs = processingFuncPtrs;
}
