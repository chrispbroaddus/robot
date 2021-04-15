
#pragma once

#include "pixel_layout.h"

namespace core {

template <ImageType IMAGE_T> struct ImageView : protected PixelLayout<IMAGE_T> {

    using typename PixelLayout<IMAGE_T>::scalar_t;
    using typename PixelLayout<IMAGE_T>::pixel_t;

    ///
    /// Number of rows in the container defined at construction time.
    ///
    size_t rows;

    ///
    /// Number of columns in the container defined at construction time.
    ///
    size_t cols;

    ///
    /// Number of SCALARS to next image row.
    ///
    size_t stride;

    ///
    /// Pointer to the image container memory.
    ///
    scalar_t* data;

    ImageView()
        : rows(0)
        , cols(0)
        , stride(0)
        , data(0) {}
    ImageView(const ImageView& copyFrom)
        : rows(copyFrom.rows)
        , cols(copyFrom.cols)
        , stride(copyFrom.stride)
        , data(copyFrom.data) {}
    ImageView(const size_t rows_, const size_t cols_, const size_t stride_, scalar_t* data_)
        : rows(rows_)
        , cols(cols_)
        , stride(stride_)
        , data(data_) {}

    pixel_t& at(const size_t row, const size_t col) { return PixelLayout<IMAGE_T>::at(*this, row, col); }
    const pixel_t& at(const size_t row, const size_t col) const { return PixelLayout<IMAGE_T>::at(*this, row, col); }
};
}
