#include "packages/image_codec/include/png/png_encoder.h"
#include "glog/logging.h"

namespace image_codec {

PngEncoder::PngEncoder(int compressionLevel)
    : m_compressionLevel(compressionLevel) {}

namespace {
    /// Custom handler for writing png data to memory
    static void pngWriteChunk(png_structp png_ptr, png_bytep data, png_size_t length) {
        std::vector<uint8_t>* p = (std::vector<uint8_t>*)png_get_io_ptr(png_ptr);
        uint8_t* pdata = (uint8_t*)data;
        p->insert(p->end(), pdata, pdata + length);
    }
}

bool PngEncoder::encode(const hal::Image& uncompressedImage, hal::Image& compressedImage) {

    std::vector<uint8_t> outputBuffer;

    /// Get input image details
    size_t height = uncompressedImage.rows();
    size_t stride = uncompressedImage.stride();
    png_byte depth = 8;
    if (uncompressedImage.type() != hal::PB_UNSIGNED_BYTE && uncompressedImage.type() != hal::PB_BYTE) {
        LOG(ERROR) << "PNG encoder does not support image type";
        throw std::runtime_error("PNG encoder does not support image type");
    }

    png_structp pngStructp = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!pngStructp) {
        LOG(ERROR) << "PNG encoder failed at png_create_write_struct";
        return false;
    }

    png_infop infoPtr = png_create_info_struct(pngStructp);
    if (!infoPtr) {
        LOG(ERROR) << "PNG encoder failed at png_create_info_struct";
        png_destroy_write_struct(&pngStructp, NULL);
        return false;
    }

    /// Set return point for error handler
    if (setjmp(png_jmpbuf(pngStructp))) {
        LOG(ERROR) << "PNG encoder failed at setjmp: setup";
        return false;
    }

    png_byte colorSpace = PNG_COLOR_TYPE_GRAY;
    if (uncompressedImage.format() == hal::PB_LUMINANCE || uncompressedImage.format() == hal::PB_RAW) {
        colorSpace = PNG_COLOR_TYPE_GRAY;
    } else if (uncompressedImage.format() == hal::PB_RGB) {
        colorSpace = PNG_COLOR_TYPE_RGB;
    } else if (uncompressedImage.format() == hal::PB_RGBA) {
        colorSpace = PNG_COLOR_TYPE_RGBA;
    } else {
        LOG(ERROR) << "PNG encoder does not support image format";
        throw std::runtime_error("PNG encoder does not support image format");
    }

    // Setup the png image header
    png_set_IHDR(pngStructp, infoPtr, uncompressedImage.cols(), uncompressedImage.rows(), depth, colorSpace, PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_set_compression_level(pngStructp, m_compressionLevel);

    /// Update return point for error handler
    if (setjmp(png_jmpbuf(pngStructp))) {
        LOG(ERROR) << "PNG encoder failed at setjmp: encode";
        return false;
    }

    /// Encode image data
    /// Create an array of row pointers
    std::vector<unsigned char*> rows(height);
    for (size_t y = 0; y < height; ++y) {
        rows[y] = (unsigned char*)uncompressedImage.data().data() + y * stride;
    }
    png_set_rows(pngStructp, infoPtr, &rows[0]);
    /// Write encoded png image into memory
    png_set_write_fn(pngStructp, &outputBuffer, pngWriteChunk, NULL);
    png_write_png(pngStructp, infoPtr, PNG_TRANSFORM_IDENTITY, NULL);

    /// Update return point for error handler
    if (setjmp(png_jmpbuf(pngStructp))) {
        LOG(ERROR) << "PNG encoder failed at setjmp: destroy";
        return false;
    }

    /// Cleanup
    png_destroy_write_struct(&pngStructp, &infoPtr);

    /// Update the Image object with the decoded image data
    compressedImage.set_data(outputBuffer.data(), outputBuffer.size());
    compressedImage.set_rows(uncompressedImage.rows());
    compressedImage.set_cols(uncompressedImage.cols());
    compressedImage.set_stride(0);
    compressedImage.set_type(hal::PB_UNSIGNED_BYTE);
    compressedImage.set_format(hal::PB_COMPRESSED_PNG);
    compressedImage.mutable_info()->CopyFrom(uncompressedImage.info());
    outputBuffer.clear();

    return true;
}

PngEncoder::~PngEncoder() {}
}
