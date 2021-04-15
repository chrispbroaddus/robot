
#include "packages/hal/include/drivers/cameras/playback/playback_camera_device.h"
#include "packages/image_codec/include/jpeg/jpeg_decoder.h"
#include "packages/image_codec/include/passthrough/passthrough_decoder.h"
#include "packages/image_codec/include/png/png_decoder.h"
#include "packages/serialization/include/protobuf_io.h"

#include "glog/logging.h"
#include <algorithm>
#include <dirent.h>
#include <thread>
#include <vector>

using namespace hal;

std::vector<std::string> getFileList(const std::string& dir) {
    DIR* dpdf;
    struct dirent* epdf;
    std::vector<std::string> files;

    dpdf = opendir(dir.c_str());
    if (dpdf != NULL) {
        while ((epdf = readdir(dpdf))) {
            std::string filename(epdf->d_name);
            if (filename.size() >= 9 && filename.substr(filename.size() - 9) == ".protodat") {
                files.push_back(filename);
            }
        }
    }

    std::sort(files.begin(), files.end());

    return files;
}

PlaybackCameraDevice::PlaybackCameraDevice(const std::string& path, uint32_t frameDelayInMilliseconds)
    : m_path(path)
    , m_frameDelayInMilliseconds(frameDelayInMilliseconds)
    , m_fileIndex(0) {

    LOG(INFO) << "Playback camera loading directory: " << path;

    m_files = getFileList(m_path);
}

bool PlaybackCameraDevice::capture(CameraSample& cameraSample) {

    std::this_thread::sleep_for(std::chrono::milliseconds(m_frameDelayInMilliseconds));

    if (m_protobufReader.get()) {
        const bool ret = m_protobufReader->readNext(cameraSample);
        if (ret) {
            if (m_decoder.get()) {
                return m_decoder->decode(cameraSample.image(), *(cameraSample.mutable_image()));
            } else {
                return true;
            }
        } else {
            m_inputFileStream.close();
        }
    }

    if (m_fileIndex == m_files.size()) {
        LOG(INFO) << "Reached end of playback data";
        return false;
    }

    const std::string filepath = m_path + "/" + m_files[m_fileIndex++];

    m_inputFileStream.open(filepath, std::ios::in | std::ios::binary);
    if (!m_inputFileStream.is_open()) {
        throw std::runtime_error("unable to open playback file");
    }
    m_protobufReader = std::unique_ptr<serialization::ProtobufReader>(new serialization::ProtobufReader(&m_inputFileStream));

    if (!m_protobufReader->readNext(cameraSample)) {
        LOG(ERROR) << "PlaybackCamera: Read image failed";
        throw std::runtime_error("PlaybackCamera: Read image failed");
    }
    if (cameraSample.image().format() == hal::PB_COMPRESSED_JPEG) {
        m_decoder.reset(new image_codec::JpegDecoder());
    } else if (cameraSample.image().format() == hal::PB_COMPRESSED_PNG) {
        m_decoder.reset(new image_codec::PngDecoder());
    } else {
        LOG(INFO) << "Playback Camera: Image is not stored in a recognized compressed image format. Using default passthrough decoder";
        m_decoder.reset((new image_codec::PassthroughDecoder()));
    }

    return true;
}
