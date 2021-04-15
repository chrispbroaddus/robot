
#pragma once

#include "image_view.h"
#include "pixel_layout.h"

namespace core {

template <ImageType IMAGE_T, typename STORAGE_T> class DynamicImage : protected PixelLayout<IMAGE_T> {
public:
    using typename PixelLayout<IMAGE_T>::pixel_t;

    typedef STORAGE_T storage_t;
    typedef ImageView<IMAGE_T> image_view_t;

    DynamicImage() = default;
    DynamicImage(const DynamicImage& copyFrom)
        : storage_(copyFrom.storage_)
        , view_(copyFrom.view_) {}
    DynamicImage(const size_t rows, const size_t cols)
        : storage_(rows * cols * sizeof(pixel_t))
        , view_(rows, cols, cols * sizeof(pixel_t), storage_.data()) {}
    DynamicImage(const size_t rows, const size_t cols, const size_t stride)
        : storage_(rows * stride)
        , view_(rows, cols, stride, storage_.data()) {}
    DynamicImage(const size_t rows, const size_t cols, const size_t stride, pixel_t* data)
        : storage_(rows * stride, data)
        , view_(rows, cols, stride, storage_.data()) {}
    ~DynamicImage() = default;

    ///
    /// @return Number of rows.
    ///
    const size_t& rows() const { return view_.rows; }

    ///
    /// @return Number of columns.
    ///
    const size_t& cols() const { return view_.cols; }

    ///
    /// @return Number of SCALARS to next image row.
    ///
    const size_t& stride() const { return view_.stride; }

    ///
    /// @return View of the entire image.
    ///
    image_view_t view() { return view_; }
    const image_view_t view() const { return view_; }

    ///
    /// @return View of a sub-image.
    ///
    image_view_t view(const size_t row, const size_t col, const size_t viewRows, const size_t viewCols) {
        assert(row < view_.rows);
        assert(col < view_.cols);
        assert(row + viewRows <= view_.rows);
        assert(col + viewCols <= view_.cols);
        return image_view_t(viewRows, viewCols, view_.stride, view_.storage + row * view_.stride + col);
    }

private:
    storage_t storage_;
    image_view_t view_;
};
}
