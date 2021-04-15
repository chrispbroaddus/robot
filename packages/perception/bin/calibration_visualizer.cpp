#include <math.h>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <unistd.h>

#include "gflags/gflags.h"

#include "packages/calibration/proto/system_calibration.pb.h"
#include "packages/perception/thirdparty/Camera.h"
#include "packages/perception/thirdparty/Interactor.h"
#include "packages/perception/visualization.h"
#include "packages/planning/proto_helpers.h"
#include "packages/planning/utils.h"

using namespace glm;
using perception::visualization::Grid;
using perception::visualization::Camera;
using perception::visualization::CoordinateSystem;
using perception::visualization::Interactor;
using perception::visualization::Primitive;

DEFINE_string(system_calibration, "", "System calibration (json) to render");

int main(int argc, char** argv) {
    gflags::SetUsageMessage("Calibration visualizer");
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    CHECK(!FLAGS_system_calibration.empty()) << "Require system calibration";
    calibration::SystemCalibration system_calibration;
    CHECK(planning::loadOptions(FLAGS_system_calibration, system_calibration, planning::SerializationType::JSON));

    CHECK(SDL_Init(SDL_INIT_VIDEO) >= 0);
    CHECK(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1) == 0);
    CHECK(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4 * 4) == 0);

    SDL_Renderer* displayRenderer = nullptr;
    SDL_Window* displayWindow = nullptr;
    SDL_RendererInfo displayRendererInfo;
    const int width = 1024;
    const int height = 768;
    SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_OPENGL, &displayWindow, &displayRenderer);
    SDL_GetRendererInfo(displayRenderer, &displayRendererInfo);
    if ((displayRendererInfo.flags & SDL_RENDERER_ACCELERATED) == 0 || (displayRendererInfo.flags & SDL_RENDERER_TARGETTEXTURE) == 0) {
        LOG(FATAL) << "Unsupported render mode";
    }
    CHECK_NOTNULL(displayRenderer);
    CHECK_NOTNULL(displayWindow);

    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        LOG(FATAL) << "Failed to initialize GLEW";
    }

    glClearColor(1.0f, 1.0f, 1.0f, 0.2f);
    glEnable(GL_DEPTH_TEST);
    checkGLErrorState();

    Camera camera;
    camera.reset();
    Interactor interactor;
    interactor.setCamera(&camera);
    interactor.setScreenSize(width, height);

    std::vector<Primitive*> primitives;

    // Base grid
    Grid grid;
    primitives.push_back(&grid);

    // Sensors
    std::vector<CoordinateSystem> sensors;
    for (const auto& cameraCalibration : *system_calibration.mutable_devicetodevicecoordinatetransformation()) {
        Sophus::SE3d extrinsic, openGLExtrinsic;
        planning::protoToPose(cameraCalibration, extrinsic);
        planning::zippyToOpenGL(extrinsic.inverse(), openGLExtrinsic);
        CoordinateSystem sensor(0.25);
        sensor.update(openGLExtrinsic);
        sensors.emplace_back(sensor);
    }
    for (const auto& coordinateSystem : sensors) {
        primitives.push_back(const_cast<Primitive*>(dynamic_cast<const Primitive*>(&coordinateSystem)));
    }

    CoordinateSystem origin;
    primitives.push_back(&origin);

    glm::mat4 projection = glm::perspectiveFov(70.0f, static_cast<float>(width), static_cast<float>(height), 0.1f, 100.0f);

    bool quit = false;
    bool primary_pressed = false;
    SDL_Event event;
    do {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    primary_pressed = true;
                }
                if (event.button.button == SDL_BUTTON_RIGHT) {
                }
            }
            if (event.type == SDL_MOUSEBUTTONUP) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    primary_pressed = false;
                }
                if (event.button.button == SDL_BUTTON_RIGHT) {
                }
            }
            if (event.type == SDL_MOUSEWHEEL) {
                interactor.setScrollDirection(event.wheel.y > 0);
            }
        }

        if (primary_pressed) {
            interactor.setLeftClicked(true);
            int xpos, ypos;
            SDL_GetMouseState(&xpos, &ypos);
            interactor.setClickPoint(xpos, ypos);
        } else {
            interactor.setLeftClicked(false);
        }
        interactor.update();

        auto model = glm::mat4();
        auto view = camera.getMatrix();
        auto MVP = projection * view * model;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (auto* primitive : primitives) {
            primitive->update(MVP);
            checkGLErrorState();
            primitive->draw();
            checkGLErrorState();
        }
        SDL_GL_SwapWindow(displayWindow);

    } while (!quit);

    return EXIT_SUCCESS;
}
