#include "packages/image_codec/include/jpeg/jpeg_encoder.h"
#include "glog/logging.h"

#include <setjmp.h>

namespace image_codec {

/// Custom error manager structure to allow the use of custom error handlers
struct JpegEncoderErrorMgr {
    struct jpeg_error_mgr pub; // "public" fields (default error manager structure)
    jmp_buf setjmp_buffer; // for return to caller
};

namespace {

    static const int kBlockSize = 16384;

    // called from libjpeg to initialize output buffer
    static void InitOutputBuffer(j_compress_ptr cinfo) {
        std::vector<uint8_t>* out = (std::vector<uint8_t>*)cinfo->client_data;
        out->resize(kBlockSize);
        cinfo->dest->next_output_byte = (JOCTET*)out->data();
        cinfo->dest->free_in_buffer = out->size();
    }

    // called from libjpeg to increase size of output buffer
    static boolean ExtendOutputBuffer(j_compress_ptr cinfo) {
        std::vector<uint8_t>* out = (std::vector<uint8_t>*)cinfo->client_data;
        size_t oldsize = out->size();
        out->resize(oldsize + kBlockSize);
        cinfo->dest->next_output_byte = (JOCTET*)(out->data() + oldsize);
        cinfo->dest->free_in_buffer = out->size() - oldsize;
        return true;
    }

    // called from libjpeg when output is complete
    static void CloseOutputBuffer(j_compress_ptr cinfo) {
        std::vector<uint8_t>* out = (std::vector<uint8_t>*)cinfo->client_data;
        out->resize(out->size() - cinfo->dest->free_in_buffer);
    }

    /// Custom error handler
    static void errorHandler(j_common_ptr compressionInfo) {
        /// compressionInfo->err really points to a JpegEncoderErrorMgr* struct, so coerce pointer
        JpegEncoderErrorMgr* errorMgr = (JpegEncoderErrorMgr*)compressionInfo->err;
        /// Display error message
        (*compressionInfo->err->output_message)(compressionInfo);
        /// Return control to the setjmp point
        longjmp(errorMgr->setjmp_buffer, 1);
    }
}

JpegEncoder::JpegEncoder(int quality)
    : m_quality(quality) {}

/// Setup the resusable structures for the encoder
bool JpegEncoder::setupEncoder(const hal::Image& image) {

    /// Determine the color of the input image
    int components;
    J_COLOR_SPACE color_space;
    if (image.type() != hal::PB_UNSIGNED_BYTE && image.type() != hal::PB_BYTE) {
        LOG(ERROR) << "JPEG encoder does not support image type";
        throw std::runtime_error("JPEG encoder does not support image type");
    }
    if (image.format() == hal::PB_LUMINANCE || image.format() == hal::PB_RAW) {
        color_space = JCS_GRAYSCALE;
        components = 1;
    } else if (image.format() == hal::PB_RGB) {
        color_space = JCS_RGB;
        components = 3;
    } else if (image.format() == hal::PB_RGBA) {
        color_space = JCS_EXT_RGBX;
        components = 4;
    } else {
        LOG(ERROR) << "JPEG encoder does not support image format";
        throw std::runtime_error("JPEG encoder does not support image format");
    }

    /// Make the library use our custom error handler
    m_jpegErrorMgr.reset(new JpegEncoderErrorMgr);
    m_compressionInfo.err = jpeg_std_error(&m_jpegErrorMgr->pub);
    m_jpegErrorMgr->pub.error_exit = &errorHandler;

    /// Set return point for the handler
    if (setjmp(m_jpegErrorMgr->setjmp_buffer)) {
        jpeg_destroy_compress(&m_compressionInfo);
        LOG(ERROR) << "Jpg encoder failed at setjmp: setupEncoder";
        throw std::runtime_error("Jpg encoder failed at setjmp: setupEncoder");
    }

    /// Initialize the JPEG compression object.
    jpeg_create_compress(&m_compressionInfo);

    /// Set the destination for the compressed image
    m_compressionInfo.dest = (struct jpeg_destination_mgr*)(*m_compressionInfo.mem->alloc_small)(
        (j_common_ptr)&m_compressionInfo, JPOOL_PERMANENT, sizeof(jpeg_destination_mgr));
    m_compressionInfo.client_data = &m_outputBuffer;
    m_compressionInfo.dest->init_destination = &InitOutputBuffer;
    m_compressionInfo.dest->empty_output_buffer = &ExtendOutputBuffer;
    m_compressionInfo.dest->term_destination = &CloseOutputBuffer;

    /// Provide the compression object with details of the input image
    m_compressionInfo.image_width = image.cols();
    m_compressionInfo.image_height = image.rows();
    m_compressionInfo.input_components = components; // # of color components per pixel
    m_compressionInfo.in_color_space = color_space; // colorSpace of input image

    /// Set the default parameters for jpeg compression
    jpeg_set_defaults(&m_compressionInfo);
    /// Set compression quality
    jpeg_set_quality(&m_compressionInfo, m_quality, TRUE); // limit to baseline-JPEG values

    return true;
}

bool JpegEncoder::encode(const hal::Image& uncompressedImage, hal::Image& compressedImage) {

    setupEncoder(uncompressedImage);

    m_outputBuffer.clear();

    /// Update return point for the handler
    if (setjmp(m_jpegErrorMgr->setjmp_buffer)) {
        jpeg_destroy_compress(&m_compressionInfo);
        LOG(ERROR) << "Jpg encoder failed at setjmp: encode";
        throw std::runtime_error("Jpg encoder failed at setjmp: encode");
    }

    /// Start encoding
    jpeg_start_compress(&m_compressionInfo, TRUE);

    /// Encode the jpeg image one line at a time
    JSAMPROW row_pointer[1];
    while (m_compressionInfo.next_scanline < m_compressionInfo.image_height) {
        // jpeg_write_scanlines expects an array of pointers to scanlines.
        // Here the array is only one element long, but you could pass
        // more than one scanline at a time if that's more convenient.
        const uint8_t* row = (uint8_t*)&uncompressedImage.data().data()[m_compressionInfo.next_scanline * uncompressedImage.stride()];
        row_pointer[0] = (unsigned char*)row;
        (void)jpeg_write_scanlines(&m_compressionInfo, row_pointer, 1);
    }

    /// Finish encoding
    jpeg_finish_compress(&m_compressionInfo);

    /// Update the Image object with the decoded image data
    compressedImage.set_data(m_outputBuffer.data(), m_outputBuffer.size());
    compressedImage.set_rows(uncompressedImage.rows());
    compressedImage.set_cols(uncompressedImage.cols());
    compressedImage.set_stride(0);
    compressedImage.set_type(hal::PB_UNSIGNED_BYTE);
    compressedImage.set_format(hal::PB_COMPRESSED_JPEG);
    compressedImage.mutable_info()->CopyFrom(uncompressedImage.info());

    /// Release JPEG compression object
    jpeg_destroy_compress(&m_compressionInfo);

    return true;
}

JpegEncoder::~JpegEncoder() {}
}
