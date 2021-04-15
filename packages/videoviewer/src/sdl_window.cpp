
#include "packages/videoviewer/include/sdl_window.h"
#include "packages/core/include/aligned_memory_allocator.h"
#include "packages/core/include/dynamic_image.h"
#include "packages/core/include/dynamic_storage.h"

#include "glog/logging.h"

#include <cmath>
#include <thread>

namespace details {
typedef core::DynamicStorage<core::rgb8_scalar_t, core::AlignedMemoryAllocator> rgb8_aligned_storage_t;
typedef core::DynamicImage<core::ImageType::rgb8, details::rgb8_aligned_storage_t> rgb8_image_t;
}

namespace video_viewer {

static void ConvertHalImageToRGB8(details::rgb8_image_t& outputImage, const hal::Image& inputImage) {

    CHECK(inputImage.type() == hal::PB_BYTE || inputImage.type() == hal::PB_UNSIGNED_BYTE || inputImage.type() == hal::PB_FLOAT);
    CHECK(inputImage.format() == hal::PB_LUMINANCE || inputImage.format() == hal::PB_RGB || inputImage.format() == hal::PB_RGBA
        || inputImage.format() == hal::PB_RANGE);

    const size_t inputCols = inputImage.cols();
    const size_t inputRows = inputImage.rows();
    const size_t inputStride = inputImage.stride();
    size_t inputChannels;

    if (inputImage.format() == hal::PB_LUMINANCE) {
        inputChannels = 1;
    } else if (inputImage.format() == hal::PB_RGB) {
        inputChannels = 3;
    } else if (inputImage.format() == hal::PB_RGBA) {
        inputChannels = 4;
    } else if (inputImage.format() == hal::PB_RANGE) {
        inputChannels = 1;
    } else {
        LOG(ERROR) << "Cannot create SDL Image: Invalid Format";
        throw std::runtime_error("Cannot create SDL Image: Invalid Format");
    }

    outputImage = details::rgb8_image_t(inputRows, inputCols);

    const size_t outputStride = outputImage.stride();
    const size_t outputChannels = 3;
    unsigned char* outputPtr = outputImage.view().data;

    if (inputImage.type() == hal::PB_BYTE || inputImage.type() == hal::PB_UNSIGNED_BYTE) {
        const unsigned char* inputPtr = (const unsigned char*)(inputImage.data().data());
        for (size_t row = 0; row < inputRows; row++) {

            const unsigned char* inputRowPtr = &inputPtr[row * inputStride];
            unsigned char* outputRowPtr = &outputPtr[row * outputStride];

            for (size_t col = 0; col < inputCols; col++) {
                unsigned char* outputPixelValue = &outputRowPtr[col * outputChannels];

                if (inputImage.format() == hal::PB_LUMINANCE) {
                    outputPixelValue[0] = inputRowPtr[col];
                    outputPixelValue[1] = inputRowPtr[col];
                    outputPixelValue[2] = inputRowPtr[col];
                } else if (inputImage.format() == hal::PB_RGB || inputImage.format() == hal::PB_RGBA) {
                    outputPixelValue[0] = inputRowPtr[col * inputChannels + 0];
                    outputPixelValue[1] = inputRowPtr[col * inputChannels + 1];
                    outputPixelValue[2] = inputRowPtr[col * inputChannels + 2];
                }
            }
        }
    } else if (inputImage.type() == hal::PB_FLOAT) {
        CHECK(inputImage.format() == hal::PB_RANGE || inputImage.format() == hal::PB_LUMINANCE);
        const float* inputPtr = (const float*)(inputImage.data().data());
        CHECK_NOTNULL(inputPtr);
        for (size_t row = 0; row < inputRows; row++) {
            // inputStride specifies the number of bytes in a row.
            // Since we are using float pointers and not char pointers we need to adjust the stride accordingly
            const float* inputRowPtr = &inputPtr[row * inputStride / 4];
            unsigned char* outputRowPtr = &outputPtr[row * outputStride];

            for (size_t col = 0; col < inputCols; col++) {
                unsigned char* outputPixelValue = &outputRowPtr[col * outputChannels];
                float floatVal = inputRowPtr[col];
                unsigned char ucharVal = (unsigned char)(floatVal * 255);
                if (floatVal >= 0 && floatVal <= 1) {
                    outputPixelValue[0] = ucharVal;
                    outputPixelValue[1] = ucharVal;
                    outputPixelValue[2] = ucharVal;
                } else if (!std::isfinite(floatVal)) {
                    outputPixelValue[0] = 255;
                    outputPixelValue[1] = 0;
                    outputPixelValue[2] = 0;
                } else if (floatVal > 1) {
                    outputPixelValue[0] = 0;
                    outputPixelValue[1] = 255;
                    outputPixelValue[2] = 0;
                } else if (floatVal < 0) {
                    outputPixelValue[0] = 0;
                    outputPixelValue[1] = 0;
                    outputPixelValue[2] = 255;
                } else {
                    outputPixelValue[0] = 255;
                    outputPixelValue[1] = 255;
                    outputPixelValue[2] = 0;
                }
            }
        }
    }
}

SDLWindow::SDLWindow(const size_t positionX, const size_t positionY, const size_t width, const size_t height, const std::string& windowName)
    : m_windowName(windowName)
    , m_window(nullptr)
    , m_renderer(nullptr) {
    SDL_Init(SDL_INIT_VIDEO);
    m_window = SDL_CreateWindow(windowName.c_str(), positionX, positionY, (int)width, (int)height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    m_renderer = SDL_CreateRenderer(m_window, 1, SDL_RENDERER_ACCELERATED);
}

SDLWindow::~SDLWindow() {
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
}

void SDLWindow::setImage(const hal::CameraSample& cameraSample, const std::string& title) {

    if ((cameraSample.image().format() == hal::PB_LUMINANCE || cameraSample.image().format() == hal::PB_RGB
            || cameraSample.image().format() == hal::PB_RGBA || cameraSample.image().format() == hal::PB_RANGE)
        && (cameraSample.image().type() == hal::PB_BYTE || cameraSample.image().type() == hal::PB_UNSIGNED_BYTE
               || cameraSample.image().type() == hal::PB_FLOAT)) {

        details::rgb8_image_t outputImage;
        ConvertHalImageToRGB8(outputImage, cameraSample.image());

        SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(outputImage.view().data, outputImage.cols(), outputImage.rows(), 24,
            outputImage.stride(), 0x0000ff, 0x00ff00, 0xff0000, 0x000000);

        if (surface == nullptr) {
            LOG(ERROR) << "SDL_CreateRGBSurfaceFrom Error: " << SDL_GetError();
            return;
        }

        SDL_Surface* windowSurface = SDL_GetWindowSurface(m_window);
        SDL_Rect roi;
        if (windowSurface->h > surface->h && windowSurface->w > surface->w) {
            roi.h = surface->h;
            roi.w = surface->w;
            roi.x = (windowSurface->w - surface->w) / 2;
            roi.y = (windowSurface->h - surface->h) / 2;
            SDL_BlitSurface(surface, 0, windowSurface, &roi);
        } else {
            SDL_BlitScaled(surface, 0, windowSurface, 0);
        }

        SDL_UpdateWindowSurface(m_window);
        SDL_SetWindowTitle(m_window, title.c_str());

        SDL_FreeSurface(surface);
    } else {
        LOG(ERROR) << "Unsupported Image format or type";
    }

    SDL_PollEvent(nullptr);
}

void SDLWindow::setWindowPosition(const size_t positionX, const size_t positionY) const {
    SDL_SetWindowPosition(m_window, positionX, positionY);
}

size_t SDLWindow::width() const {
    int w, h;
    SDL_GetWindowSize(m_window, &w, &h);
    return (size_t)w;
}

size_t SDLWindow::height() const {
    int w, h;
    SDL_GetWindowSize(m_window, &w, &h);
    return (size_t)h;
}
}
