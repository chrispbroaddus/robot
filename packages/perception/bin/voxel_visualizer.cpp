#include <atomic>
#include <math.h>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <unistd.h>

#include "gflags/gflags.h"

#include "packages/estimation/estimator.h"
#include "packages/estimation/visualization.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/perception/grid.h"
#include "packages/perception/logging.h"
#include "packages/perception/perception.h"
#include "packages/perception/thirdparty/Camera.h"
#include "packages/perception/thirdparty/Interactor.h"
#include "packages/perception/visualization.h"
#include "packages/planning/utils.h"

DEFINE_string(voxelSubscriberAddress, "tcp://localhost:6550", "Voxel address to subscribe to");
DEFINE_string(voxelTopic, "voxels", "Voxel topic");
DEFINE_string(telemetryAddress, "tcp://localhost", "Telemetry data");
DEFINE_string(telemetryPort, "7001", "Port");

static std::atomic_bool s_active;

using namespace glm;
using perception::VoxelGridProto;
using perception::PointCloudXYZ;
using perception::visualization::Grid;
using perception::visualization::Voxels;
using perception::visualization::PointCloud;
using perception::visualization::Primitive;
using perception::visualization::CoordinateSystem;

typedef struct {
    bool redrawVoxels;
    std::mutex voxelLock;
    VoxelGridProto* grid;
    PointCloudXYZ* cloud;
    bool redrawState;
    std::mutex stateLock;
    std::deque<estimation::State> states;
} visualization_t;

void subscribeVoxels(visualization_t* visualization) {
    CHECK_NOTNULL(visualization);
    zmq::context_t context(1);
    net::ZMQProtobufSubscriber<perception::VoxelGridProto> subscriber(context, FLAGS_voxelSubscriberAddress, FLAGS_voxelTopic, 1);
    perception::VoxelGridProto grid;
    visualization->grid = &grid;
    while (s_active) {
        if (subscriber.poll()) {
            CHECK(subscriber.recv(grid));
            std::lock_guard<std::mutex> guard(visualization->voxelLock);
            visualization->redrawVoxels = true;
            *visualization->grid = grid;
        }
    }
}

void updateVisualization(
    visualization_t* visualization, Voxels* voxels, PointCloud* cloud, estimation::visualization::PoseVisualizer* pose) {
    CHECK_NOTNULL(visualization);
    CHECK_NOTNULL(voxels);
    CHECK_NOTNULL(cloud);
    CHECK_NOTNULL(pose);
    if (visualization->grid == nullptr) {
        return;
    }
    if (visualization->redrawVoxels) {
        if (visualization->voxelLock.try_lock()) {
            CHECK_NOTNULL(visualization->grid);
            voxels->update(*visualization->grid);
            visualization->redrawVoxels = false;
        }
        visualization->voxelLock.unlock();
    }

    if (visualization->redrawState) {
        if (visualization->stateLock.try_lock()) {
            pose->updateStates(visualization->states);
            visualization->redrawState = false;
            visualization->stateLock.unlock();
        }
    }
}

int main(int argc, char** argv) {
    gflags::SetUsageMessage("Voxel visualizer");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    const int width = 1024;
    const int height = 768;

    CHECK(SDL_Init(SDL_INIT_VIDEO) >= 0);
    CHECK(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) == 0);

    CHECK(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1) == 0);
    CHECK(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4 * 4) == 0);

    SDL_Renderer* displayRenderer = nullptr;
    SDL_Window* displayWindow = nullptr;
    SDL_RendererInfo displayRendererInfo;
    SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_OPENGL, &displayWindow, &displayRenderer);
    SDL_GetRendererInfo(displayRenderer, &displayRendererInfo);
    if ((displayRendererInfo.flags & SDL_RENDERER_ACCELERATED) == 0 || (displayRendererInfo.flags & SDL_RENDERER_TARGETTEXTURE) == 0) {
        LOG(FATAL) << "Unsupported render mode";
    }

    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        LOG(FATAL) << "Failed to initialize GLEW";
    }

    glClearColor(1.0f, 1.0f, 1.0f, 0.2f);
    glEnable(GL_DEPTH_TEST);
    checkGLErrorState();

    s_active = true;

    using perception::visualization::Grid;

    glm::mat4 projection = glm::perspectiveFov(70.0f, static_cast<float>(width), static_cast<float>(height), 0.1f, 100.0f);

    using perception::visualization::Camera;
    using perception::visualization::Interactor;
    Camera camera;
    camera.reset();
    Interactor interactor;
    interactor.setCamera(&camera);
    interactor.setScreenSize(width, height);

    std::vector<Primitive*> primitives;

    // Grid
    Grid grid;
    primitives.push_back(&grid);

    // Voxels
    Voxels voxels;
    primitives.push_back(&voxels);

    // Received cloud
    PointCloud cloud;
    primitives.push_back(&cloud);

    // Origin
    CoordinateSystem origin;
    primitives.push_back(&origin);

    // Poses
    estimation::visualization::PoseVisualizer poseVisualizer;
    primitives.push_back(&poseVisualizer);

    visualization_t visualization;
    visualization.grid = nullptr;
    visualization.cloud = nullptr;

    std::deque<std::thread> workers;
    workers.emplace_back(std::thread(subscribeVoxels, &visualization));

    bool primary_pressed = false;
    SDL_Event event;

    do {
        updateVisualization(&visualization, &voxels, &cloud, &poseVisualizer);

        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                s_active = false;
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
    } while (s_active);
    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    return EXIT_SUCCESS;
}
