#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stddef.h>

namespace core {

enum class ImageType { uint8 = 0, int16, int32, float32, rgb8, rgba8, bgr8 };

struct rgb8_pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct rgba8_pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct bgr8_pixel {
    uint8_t b;
    uint8_t g;
    uint8_t r;
};

template <ImageType IMAGE_TYPE> struct PixelLayout { static constexpr ImageType type = IMAGE_TYPE; };

template <> struct PixelLayout<ImageType::uint8> {

    typedef uint8_t scalar_t;
    typedef uint8_t pixel_t;

    template <typename IMAGE_VIEW_T> static pixel_t& at(IMAGE_VIEW_T& view, const size_t row, const size_t col) {
        assert(row < view.rows);
        assert(col < view.cols);
        return view.data[row * view.stride + col];
    }
};

template <> struct PixelLayout<ImageType::int16> {

    typedef int16_t scalar_t;
    typedef int16_t pixel_t;

    template <typename IMAGE_VIEW_T> static pixel_t& at(IMAGE_VIEW_T& view, const size_t row, const size_t col) {
        assert(row < view.rows);
        assert(col < view.cols);
        return view.data[row * view.stride + col];
    }
};

template <> struct PixelLayout<ImageType::int32> {

    typedef int32_t scalar_t;
    typedef int32_t pixel_t;

    template <typename IMAGE_VIEW_T> static pixel_t& at(IMAGE_VIEW_T& view, const size_t row, const size_t col) {
        assert(row < view.rows);
        assert(col < view.cols);
        return view.data[row * view.stride + col];
    }
};

template <> struct PixelLayout<ImageType::float32> {

    typedef float scalar_t;
    typedef float pixel_t;

    template <typename IMAGE_VIEW_T> static pixel_t& at(IMAGE_VIEW_T& view, const size_t row, const size_t col) {
        assert(row < view.rows);
        assert(col < view.cols);
        return view.data[row * view.stride + col];
    }
};

template <> struct PixelLayout<ImageType::rgb8> {

    typedef uint8_t scalar_t;
    typedef rgb8_pixel pixel_t;

    template <typename IMAGE_VIEW_T> static pixel_t& at(IMAGE_VIEW_T& view, const size_t row, const size_t col) {
        assert(row < view.rows);
        assert(col < view.cols);
        return reinterpret_cast<pixel_t&>(view.data[row * view.stride + 3 * col]);
    }
};

template <> struct PixelLayout<ImageType::rgba8> {

    typedef uint8_t scalar_t;
    typedef rgb8_pixel pixel_t;

    template <typename IMAGE_VIEW_T> static pixel_t& at(IMAGE_VIEW_T& view, const size_t row, const size_t col) {
        assert(row < view.rows);
        assert(col < view.cols);
        return reinterpret_cast<pixel_t&>(view.data[row * view.stride + 4 * col]);
    }
};

template <> struct PixelLayout<ImageType::bgr8> {

    typedef uint8_t scalar_t;
    typedef bgr8_pixel pixel_t;

    template <typename IMAGE_VIEW_T> static pixel_t& at(IMAGE_VIEW_T& view, const size_t row, const size_t col) {
        assert(row < view.rows);
        assert(col < view.cols);
        return reinterpret_cast<pixel_t&>(view.data[row * view.stride + 3 * col]);
    }
};

typedef PixelLayout<ImageType::uint8>::scalar_t uint8_scalar_t;
typedef PixelLayout<ImageType::uint8>::pixel_t uint8_pixel_t;

typedef PixelLayout<ImageType::int16>::scalar_t int16_scalar_t;
typedef PixelLayout<ImageType::int16>::pixel_t int16_pixel_t;

typedef PixelLayout<ImageType::int32>::scalar_t int32_scalar_t;
typedef PixelLayout<ImageType::int32>::pixel_t int32_pixel_t;

typedef PixelLayout<ImageType::float32>::scalar_t float32_scalar_t;
typedef PixelLayout<ImageType::float32>::pixel_t float32_pixel_t;

typedef PixelLayout<ImageType::rgb8>::scalar_t rgb8_scalar_t;
typedef PixelLayout<ImageType::rgb8>::pixel_t rgb8_pixel_t;

typedef PixelLayout<ImageType::rgba8>::scalar_t rgba8_scalar_t;
typedef PixelLayout<ImageType::rgba8>::pixel_t rgba8_pixel_t;

typedef PixelLayout<ImageType::bgr8>::scalar_t bgr8_scalar_t;
typedef PixelLayout<ImageType::bgr8>::pixel_t bgr8_pixel_t;
}
