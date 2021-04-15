#include "packages/image_codec/include/jpeg.h"

#include <csetjmp>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "jpeglib.h"

#include "glog/logging.h"

namespace image_codec {

bool EncodeJPEG(const core::ImageView<core::ImageType::uint8>& image, int quality, std::vector<uint8_t>* out) {
    return EncodeJPEG(image.data, image.cols, image.rows, image.stride * sizeof(uint8_t), core::ImageType::uint8, quality, out);
}

bool EncodeJPEG(const core::ImageView<core::ImageType::rgb8>& image, int quality, std::vector<uint8_t>* out) {
    return EncodeJPEG(image.data, image.cols, image.rows, image.stride * sizeof(core::rgb8_pixel), core::ImageType::rgb8, quality, out);
}

bool EncodeJPEG(const core::ImageView<core::ImageType::rgba8>& image, int quality, std::vector<uint8_t>* out) {
    return EncodeJPEG(image.data, image.cols, image.rows, image.stride * sizeof(core::rgba8_pixel), core::ImageType::rgba8, quality, out);
}

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
}

// Encode a grayscale image as a JPEG and return the result in the provided vector.
// quality is in the range 1..100 where smaller values mean smaller output sizes.
bool EncodeJPEG(
    const uint8_t* data, size_t width, size_t height, size_t stride, core::ImageType layout, int quality, std::vector<uint8_t>* out) {

    // deal with color space
    int components;
    J_COLOR_SPACE color_space;
    switch (layout) {
    case core::ImageType::uint8:
        color_space = JCS_GRAYSCALE;
        components = 1;
        break;
    case core::ImageType::rgb8:
        color_space = JCS_RGB;
        components = 3;
        break;
    case core::ImageType::rgba8:
        color_space = JCS_EXT_RGBX;
        components = 4;
        break;
    default:
        LOG(ERROR) << "JPEG encoder does not support image type";
        return false;
    }

    // reset the output buffer
    out->clear();

    // This struct contains the JPEG compression parameters and pointers to
    // working space (which is allocated as needed by the JPEG library).
    // It is possible to have several such structures, representing multiple
    // compression/decompression processes, in existence at once.  We refer
    // to any one struct (and its associated working data) as a "JPEG object".
    struct jpeg_compress_struct cinfo;

    // This struct represents a JPEG error handler.  It is declared separately
    // because applications often want to supply a specialized error handler
    // (see the second half of this file for an example).  But here we just
    // take the easy way out and use the standard error handler, which will
    // print a message on stderr and call exit() if compression fails.
    // Note that this struct must live as long as the main JPEG parameter
    // struct, to avoid dangling-pointer problems.
    struct jpeg_error_mgr jerr;

    // Step 1: allocate and initialize JPEG compression object

    // We have to set up the error handler first, in case the initialization
    // step fails.  (Unlikely, but it could happen if you are out of memory.)
    // This routine fills in the contents of struct jerr, and returns jerr's
    // address which we place into the link field in cinfo.
    cinfo.err = jpeg_std_error(&jerr);
    // Now we can initialize the JPEG compression object.
    jpeg_create_compress(&cinfo);

    // Step 2: specify data destination (eg, a file)

    // Note: steps 2 and 3 can be done in either order.
    cinfo.dest
        = (struct jpeg_destination_mgr*)(*cinfo.mem->alloc_small)((j_common_ptr)&cinfo, JPOOL_PERMANENT, sizeof(jpeg_destination_mgr));

    cinfo.client_data = out;
    cinfo.dest->init_destination = &InitOutputBuffer;
    cinfo.dest->empty_output_buffer = &ExtendOutputBuffer;
    cinfo.dest->term_destination = &CloseOutputBuffer;
    // jpeg_stdio_dest(&cinfo, outfile);

    // Step 3: set parameters for compression

    // First we supply a description of the input image.
    // Four fields of the cinfo struct must be filled in:
    cinfo.image_width = width; // image width and height, in pixels
    cinfo.image_height = height;
    cinfo.input_components = components; // # of color components per pixel
    cinfo.in_color_space = color_space; // colorspace of input image
    // Now use the library's routine to set default compression parameters.
    // (You must set at least cinfo.in_color_space before calling this,
    // since the defaults depend on the source color space.)
    jpeg_set_defaults(&cinfo);
    // Now you can set any non-default parameters you wish to.
    // Here we just illustrate the use of quality (quantization table) scaling:
    jpeg_set_quality(&cinfo, quality, TRUE); // limit to baseline-JPEG values

    // Step 4: Start compressor

    // TRUE ensures that we will write a complete interchange-JPEG file.
    // Pass TRUE unless you are very sure of what you're doing.
    jpeg_start_compress(&cinfo, TRUE);

    // Step 5: while (scan lines remain to be written)
    //           jpeg_write_scanlines(...);

    // Here we use the library's state variable cinfo.next_scanline as the
    // loop counter, so that we don't have to keep track ourselves.
    // To keep things simple, we pass one scanline per call; you can pass
    // more if you wish, though.
    JSAMPROW row_pointer[1];
    while (cinfo.next_scanline < cinfo.image_height) {
        // jpeg_write_scanlines expects an array of pointers to scanlines.
        // Here the array is only one element long, but you could pass
        // more than one scanline at a time if that's more convenient.
        const uint8_t* row = &data[cinfo.next_scanline * stride];
        row_pointer[0] = (unsigned char*)row;
        (void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    // Step 6: Finish compression
    jpeg_finish_compress(&cinfo);

    // Step 7: release JPEG compression object
    jpeg_destroy_compress(&cinfo);

    // And we're done!
    return true;
}

} // namespace image_codec
