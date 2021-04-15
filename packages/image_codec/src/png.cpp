#include "packages/image_codec/include/png.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <png.h>
#include <vector>

#include "glog/logging.h"

namespace image_codec {

bool EncodePNG(const core::ImageView<core::ImageType::uint8>& image, int compression, std::vector<uint8_t>* out) {
    return EncodePNG(image.data, image.cols, image.rows, image.stride * sizeof(uint8_t), core::ImageType::uint8, compression, out);
}

bool EncodePNG(const core::ImageView<core::ImageType::rgb8>& image, int compression, std::vector<uint8_t>* out) {
    return EncodePNG(image.data, image.cols, image.rows, image.stride * sizeof(core::rgb8_pixel), core::ImageType::rgb8, compression, out);
}

namespace {
    static void PNGWriteChunk(png_structp png_ptr, png_bytep data, png_size_t length) {
        std::vector<uint8_t>* p = (std::vector<uint8_t>*)png_get_io_ptr(png_ptr);
        uint8_t* pdata = (uint8_t*)data;
        p->insert(p->end(), pdata, pdata + length);
    }
}

bool EncodePNG(
    const uint8_t* data, size_t width, size_t height, size_t stride, core::ImageType layout, int compression, std::vector<uint8_t>* out) {
    out->clear();

    // deal with color space
    png_byte depth;
    png_byte color_space;
    switch (layout) {
    case core::ImageType::uint8:
        color_space = PNG_COLOR_TYPE_GRAY;
        depth = 8;
        break;
    case core::ImageType::rgb8:
        color_space = PNG_COLOR_TYPE_RGB;
        depth = 24;
        break;
    default:
        LOG(ERROR) << "PNG encoder does not support image type";
        return false;
    }

    // initialize structure
    png_structp p = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!p) {
        LOG(ERROR) << "EncodePNG failed at png_create_write_struct";
        return false;
    }

    // create info struct
    png_infop info_ptr = png_create_info_struct(p);
    if (!info_ptr) {
        LOG(ERROR) << "EncodePNG failed at png_create_info_struct";
        return false;
    }

    if (setjmp(png_jmpbuf(p))) {
        LOG(ERROR) << "EncodePNG failed at setjmp";
        return false;
    }

    // setup header
    png_set_IHDR(p, info_ptr, width, height, depth, color_space, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_set_compression_level(p, compression);

    // create row pointers
    std::vector<unsigned char*> rows(height);
    for (size_t y = 0; y < height; ++y) {
        rows[y] = (unsigned char*)data + y * stride;
    }
    png_set_rows(p, info_ptr, &rows[0]);

    // write data
    png_set_write_fn(p, out, PNGWriteChunk, NULL);
    png_write_png(p, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    // cleanup
    png_destroy_write_struct(&p, NULL);
    return true;
}

} // namespace image_codec
