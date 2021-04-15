#include "packages/image_codec/include/png/png_decoder.h"
#include "glog/logging.h"

namespace image_codec {

namespace {
    /// Create a custom data structure to read png data from memory
    struct ReadBuffer {
        unsigned char* buffer;
        size_t bytesRead;
    };
    /// Custom handler for reading png data from memory
    static void pngReadChunk(png_structp png_ptr, png_bytep data, png_size_t length) {
        struct ReadBuffer* readBuffer = (struct ReadBuffer*)png_get_io_ptr(png_ptr);
        if (readBuffer == NULL || readBuffer->buffer == NULL) {
            LOG(ERROR) << "Invalid png data source";
            throw std::runtime_error("Invalid png data source");
        }
        if (data == NULL) {
            LOG(INFO) << "Null";
        }
        memcpy(data, readBuffer->buffer + readBuffer->bytesRead, length);
        readBuffer->bytesRead += length;
    }
}

PngDecoder::PngDecoder() {}

bool PngDecoder::decode(const hal::Image& compressedImage, hal::Image& uncompressedImage) {

    if (compressedImage.format() != hal::PB_COMPRESSED_PNG) {
        LOG(ERROR) << "PngDecoder: Input image is not in png format";
        throw std::runtime_error("PngDecoder: Input image is not in png format");
    }

    png_structp pngStructp = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!pngStructp) {
        LOG(ERROR) << "Cannot create png read structure";
        throw std::runtime_error("Cannot create png read structure");
    }

    png_infop infoPtr = png_create_info_struct(pngStructp);
    if (!infoPtr) {
        png_destroy_read_struct(&pngStructp, NULL, NULL);
        LOG(ERROR) << "Cannot create png info structure";
        throw std::runtime_error("Cannot create png info structure");
    }

    /// Set return point for error handler
    if (setjmp(png_jmpbuf(pngStructp))) {
        LOG(ERROR) << "PNG decoder failed at setjmp: read header";
        png_destroy_read_struct(&pngStructp, &infoPtr, NULL);
        return false;
    }

    /// Initialize our custom image buffer to point to input CameraSample's image data
    struct ReadBuffer readBuffer;
    readBuffer.buffer = (unsigned char*)compressedImage.data().data();
    readBuffer.bytesRead = 0;
    png_set_read_fn(pngStructp, &readBuffer, &pngReadChunk);

    /// Read png header
    png_read_info(pngStructp, infoPtr);

    /// Get image details from the image header
    const size_t width = png_get_image_width(pngStructp, infoPtr);
    const size_t height = png_get_image_height(pngStructp, infoPtr);
    const int colorSpace = png_get_color_type(pngStructp, infoPtr);
    const int bitDepth = png_get_bit_depth(pngStructp, infoPtr);

    /// Update return point for error handler
    if (setjmp(png_jmpbuf(pngStructp))) {
        LOG(ERROR) << "PNG decoder failed at setjmp: decode";
        png_destroy_read_struct(&pngStructp, &infoPtr, NULL);
        return false;
    }

    /// Copy the image meta-data over for the uncompressed image from the compressed image
    uncompressedImage.set_rows(height);
    uncompressedImage.set_cols(width);
    uncompressedImage.set_type(hal::PB_UNSIGNED_BYTE);

    size_t stride;
    if (colorSpace == PNG_COLOR_TYPE_GRAY) {
        uncompressedImage.set_format(hal::PB_LUMINANCE);
        stride = width * bitDepth / 8;
    } else if (colorSpace == PNG_COLOR_TYPE_RGB) {
        uncompressedImage.set_format(hal::PB_RGB);
        stride = width * 3 * bitDepth / 8;
    } else if (colorSpace == PNG_COLOR_TYPE_RGBA) {
        uncompressedImage.set_format(hal::PB_RGBA);
        stride = width * 4 * bitDepth / 8;
    } else {
        LOG(ERROR) << "Unsupported un-compressed image format: " << colorSpace;
        throw std::runtime_error("Unsupported un-compressed image format");
    }
    uncompressedImage.set_stride(stride);
    uncompressedImage.mutable_info()->CopyFrom(compressedImage.info());

    /// Decode the image data
    std::vector<uint8_t> outputBuffer(height * stride);
    std::vector<unsigned char*> rows(height);
    for (size_t y = 0; y < height; ++y) {
        rows[y] = (unsigned char*)outputBuffer.data() + y * stride;
    }
    png_read_image(pngStructp, &rows[0]);

    /// Update return point for error handler
    if (setjmp(png_jmpbuf(pngStructp))) {
        LOG(ERROR) << "PNG decoder failed at setjmp: destroy";
        png_destroy_read_struct(&pngStructp, &infoPtr, NULL);
        return false;
    }

    /// Cleanup
    png_destroy_read_struct(&pngStructp, &infoPtr, NULL);

    /// Update the Image object with the decoded image data
    uncompressedImage.set_data(outputBuffer.data(), outputBuffer.size());

    return true;
}

PngDecoder::~PngDecoder() {}
}
