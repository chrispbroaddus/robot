
#pragma once

#include <vector>

#include "SDL.h"
#include "packages/hal/proto/camera_sample.pb.h"

namespace video_viewer {

struct SDLWindowState;

class SDLWindow {
public:
    SDLWindow() = delete;
    SDLWindow(const size_t positionX, const size_t positionY, const size_t width, const size_t height, const std::string& windowName);
    ~SDLWindow();

    void setImage(const hal::CameraSample& cameraSample, const std::string& title);

    size_t width() const;
    size_t height() const;

    void setWindowPosition(const size_t positionX, const size_t positionY) const;

    SDL_Window* getWindowHandle() { return m_window; }
    SDL_Renderer* getRendererHandle() { return m_renderer; }

private:
    const std::string m_windowName;
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
};
}
