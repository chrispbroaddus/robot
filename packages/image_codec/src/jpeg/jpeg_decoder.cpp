#include "packages/image_codec/include/jpeg/jpeg_decoder.h"
#include "glog/logging.h"

#include <iterator>
#include <setjmp.h>

namespace image_codec {

/// Custom error manager structure to allow the use of custom error handlers
struct JpegDecoderErrorMgr {
    struct jpeg_error_mgr pub; // "public" fields (default error manager structure)
    jmp_buf setjmp_buffer; // for return to caller
};

namespace {

    /// Custom error handler
    static void errorHandler(j_common_ptr compressionInfo) {
        /// compressionInfo->err really points to a JpegDecoderErrorMgr* struct, so coerce pointer
        JpegDecoderErrorMgr* errorMgr = (JpegDecoderErrorMgr*)compressionInfo->err;
        /// Display error message
        (*compressionInfo->err->output_message)(compressionInfo);
        /// Return control to the setjmp point
        longjmp(errorMgr->setjmp_buffer, 1);
    }
}

JpegDecoder::JpegDecoder() {

    /// Make the library use our custom error handler
    m_jpegErrorMgr.reset(new JpegDecoderErrorMgr);
    m_compressionInfo.err = jpeg_std_error(&m_jpegErrorMgr->pub);
    m_jpegErrorMgr->pub.error_exit = &errorHandler;

    /// Set return point for the handler
    if (setjmp(m_jpegErrorMgr->setjmp_buffer)) {
        jpeg_destroy_decompress(&m_compressionInfo);
        LOG(ERROR) << "Jpg decoder failed at setjmp: create";
        throw std::runtime_error("Jpg decoder failed at setjmp: create");
    }

    /// Initialize the JPEG decompression object.
    jpeg_create_decompress(&m_compressionInfo);
}

bool JpegDecoder::decode(const hal::Image& compressedImage, hal::Image& uncompressedImage) {

    if (compressedImage.format() != hal::PB_COMPRESSED_JPEG) {
        LOG(ERROR) << "JpegDecoder: Input image is not in jpeg format";
        throw std::runtime_error("JpegDecoder: Input image is not in jpeg format");
    }

    int retval;

    /// Update return point for error handler
    if (setjmp(m_jpegErrorMgr->setjmp_buffer)) {
        jpeg_destroy_decompress(&m_compressionInfo);
        LOG(ERROR) << "Jpg decoder failed at setjmp: decode";
        throw std::runtime_error("Jpg decoder failed at setjmp: decode");
    }

    /// Set the camera sample's image data as the source for the compressed image data
    jpeg_mem_src(&m_compressionInfo, (unsigned char*)compressedImage.data().data(),
        (unsigned long)std::distance(compressedImage.data().begin(), compressedImage.data().end()));

    /// Check the jpeg header to determine if the image is a valid jpeg image
    retval = jpeg_read_header(&m_compressionInfo, TRUE);
    if (retval != 1) {
        LOG(ERROR) << "Image does not seem to be a normal JPEG";
        throw std::runtime_error("Image does not seem to be a normal JPEG");
    }

    /// Start decoding
    jpeg_start_decompress(&m_compressionInfo);

    /// Fetch the decoded image information from the header
    int width = m_compressionInfo.output_width;
    int height = m_compressionInfo.output_height;
    int pixelSize = m_compressionInfo.output_components;
    int stride = width * pixelSize;
    std::vector<unsigned char> outputBuffer(width * height * pixelSize);

    /// Decode the jpeg image one line at a time
    while (m_compressionInfo.output_scanline < m_compressionInfo.output_height) {
        unsigned char* bufferArray[1];
        bufferArray[0] = outputBuffer.data() + m_compressionInfo.output_scanline * stride;
        jpeg_read_scanlines(&m_compressionInfo, bufferArray, 1);
    }

    /// Finish decoding
    jpeg_finish_decompress(&m_compressionInfo);

    /// Update the Image object with the decoded image data
    uncompressedImage.set_rows(height);
    uncompressedImage.set_cols(width);
    uncompressedImage.set_stride(stride);
    uncompressedImage.set_type(hal::PB_UNSIGNED_BYTE);
    uncompressedImage.mutable_info()->CopyFrom(compressedImage.info());
    if (pixelSize == 1) {
        uncompressedImage.set_format(hal::PB_LUMINANCE);
    } else if (pixelSize == 3) {
        uncompressedImage.set_format(hal::PB_RGB);
    } else if (pixelSize == 4) {
        uncompressedImage.set_format(hal::PB_RGBA);
    } else {
        LOG(ERROR) << "Unsupported un-compressed image format";
        throw std::runtime_error("Unsupported un-compressed image format");
    }
    uncompressedImage.set_data(outputBuffer.data(), outputBuffer.size());

    return true;
}

JpegDecoder::~JpegDecoder() {
    /// Free the decompression object
    jpeg_destroy_decompress(&m_compressionInfo);
}
}
