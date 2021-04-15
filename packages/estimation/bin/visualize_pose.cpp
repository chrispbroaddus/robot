#include <math.h>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <unistd.h>

#include "gflags/gflags.h"
#include "sophus/se3.hpp"

#include "packages/core/proto/geometry.pb.h"
#include "packages/estimation/estimator.h"
#include "packages/estimation/visualization.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/perception/thirdparty/Camera.h"
#include "packages/perception/thirdparty/Interactor.h"
#include "packages/perception/visualization.h"
#include "packages/planning/logging.h"
#include "packages/planning/utils.h"
#include "packages/unity_plugins/proto/unity_telemetry_envelope.pb.h"

DEFINE_string(address, "", "Address to connect to");

static bool s_active = true;

using namespace glm;

typedef struct {
    std::deque<estimation::State> odometryStates;
    std::mutex odometryLock;
    bool redrawOdometry;
    std::deque<estimation::State> groundTruthStates;
    std::mutex groundTruthLock;
    bool redrawGroundTruth;
} Visualization;

void visualizeGroundTruth(Visualization* visualization) {
    CHECK_NOTNULL(visualization);
    CHECK(!FLAGS_address.empty());
    const int groundTruthPort = 7101;
    std::string fullyQualifiedAddress = FLAGS_address + ":" + std::to_string(groundTruthPort);
    zmq::context_t context(1);
    net::ZMQProtobufSubscriber<estimation::StateProto> subscriber(context, fullyQualifiedAddress, "ground_truth", 1);
    estimation::State siteFrame;
    initialiseState(siteFrame);
    bool siteFrameInitialised = false;

    while (s_active) {
        if (subscriber.poll()) {
            estimation::StateProto odometryState;
            if (subscriber.recv(odometryState)) {
                Sophus::SE3d pose;
                planning::protoToPose(odometryState.transform(), pose);

                estimation::State state;
                initialiseState(state);
                state.m_pose = pose;

                if (!siteFrameInitialised) {
                    siteFrame = state;
                    siteFrameInitialised = true;
                }

                state.m_pose = siteFrame.m_pose.inverse() * state.m_pose;
                std::lock_guard<std::mutex> groundTruthLock(visualization->groundTruthLock);
                visualization->groundTruthStates.emplace_back(state);
                visualization->redrawGroundTruth = true;
            }
        }
    }
}

void visualizeOdometry(Visualization* visualization) {
    CHECK_NOTNULL(visualization);
    CHECK(!FLAGS_address.empty());
    const int odometryPort = 7100;
    std::string fullyQualifiedAddress = FLAGS_address + ":" + std::to_string(odometryPort);
    zmq::context_t context(1);
    net::ZMQProtobufSubscriber<estimation::StateProto> subscriber(context, fullyQualifiedAddress, "odometry", 1);
    estimation::State siteFrame;
    initialiseState(siteFrame);

    bool siteFrameInitialised = false;

    int counter = 0;
    while (s_active) {
        if (subscriber.poll()) {
            estimation::StateProto odometryState;
            if (subscriber.recv(odometryState) && (counter++ % 10 == 0)) {
                Sophus::SE3d pose;
                planning::protoToPose(odometryState.transform(), pose);

                estimation::State state;
                initialiseState(state);
                state.m_pose = pose;

                if (!siteFrameInitialised) {
                    siteFrame = state;
                    siteFrameInitialised = true;
                }
                state.m_pose = siteFrame.m_pose.inverse() * state.m_pose;
                std::lock_guard<std::mutex> odometryLock(visualization->odometryLock);
                visualization->odometryStates.emplace_back(state);
                visualization->redrawOdometry = true;
            }
        }
    }
}

void updateVisualization(Visualization* visualization, estimation::visualization::PoseVisualizer* odometryVisualizer,
    estimation::visualization::PoseVisualizer* groundTruthVisualizer) {
    CHECK_NOTNULL(visualization);
    CHECK_NOTNULL(odometryVisualizer);
    CHECK_NOTNULL(groundTruthVisualizer);

    if (visualization->redrawOdometry) {
        if (visualization->odometryLock.try_lock()) {
            odometryVisualizer->updateStates(visualization->odometryStates);
            visualization->redrawOdometry = false;
            visualization->odometryLock.unlock();
        }
    }

    if (visualization->redrawGroundTruth) {
        if (visualization->groundTruthLock.try_lock()) {
            groundTruthVisualizer->updateStates(visualization->groundTruthStates);
            visualization->redrawOdometry = false;
            visualization->groundTruthLock.unlock();
        }
    }
}

int main(int argc, char** argv) {
    gflags::SetUsageMessage("Pose visualizer");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    CHECK(SDL_Init(SDL_INIT_VIDEO) >= 0);
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

    glm::mat4 projection = glm::perspectiveFov(70.0f, static_cast<float>(width), static_cast<float>(height), 0.1f, 100.0f);

    using perception::visualization::Camera;
    using perception::visualization::Interactor;
    Camera camera;
    camera.reset();
    Interactor interactor;
    interactor.setCamera(&camera);
    interactor.setScreenSize(width, height);

    std::vector<perception::visualization::Primitive*> primitives;

    using perception::visualization::Grid;
    Grid grid;
    primitives.push_back(&grid);

    using estimation::visualization::PoseVisualizer;
    PoseVisualizer odometryVisualizer;
    primitives.push_back(&odometryVisualizer);
    PoseVisualizer groundTruthVisualizer;
    primitives.push_back(&groundTruthVisualizer);

    Visualization visualization;
    std::vector<std::thread> worker_threads;

    // Visualize odometry
    worker_threads.emplace_back(std::thread(visualizeOdometry, &visualization));
    // Visualize ground-truth
    worker_threads.emplace_back(std::thread(visualizeGroundTruth, &visualization));

    bool primary_pressed = false;
    SDL_Event event;
    do {
        updateVisualization(&visualization, &odometryVisualizer, &groundTruthVisualizer);

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
    for (auto& thread : worker_threads) {
        thread.join();
    }
    return EXIT_SUCCESS;
}
