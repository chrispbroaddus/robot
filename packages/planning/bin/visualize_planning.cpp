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
#include "packages/executor/proto/executor_options.pb.h"
#include "packages/executor/proto_helpers.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/perception/perception.h"
#include "packages/perception/proto/perception_options.pb.h"
#include "packages/perception/thirdparty/Camera.h"
#include "packages/perception/thirdparty/Interactor.h"
#include "packages/perception/visualization.h"
#include "packages/planning/navigator.h"
#include "packages/planning/planner.h"
#include "packages/planning/proto/trajectory_planner_options.pb.h"
#include "packages/planning/utils.h"

DEFINE_string(mode, "waypoint", "Planning mode to execute (point_and_go, waypoint)");
DEFINE_string(pose, "groundTruth", "Estimation to use (groundTruth, odometry)");

static std::atomic_bool s_active(true);

constexpr float kGoalDecisionBoundary = 0.25f;

using namespace glm;

struct VisualizationState {
    std::deque<estimation::State> groundTruthStates;
    std::atomic<bool> redrawGroundTruthStates;
    std::mutex groundTruthStatesLock;

    std::deque<estimation::State> odometryStates;
    std::atomic<bool> redrawOdometryStates;
    std::mutex odometryStatesLock;

    std::vector<core::Point2d> waypoints;
    std::atomic<bool> redrawWaypoints;
    std::mutex waypoints_lock;

    std::vector<core::Point2d> path;
    std::atomic<bool> redrawPath;
    std::mutex pathLock;

    std::vector<core::Point2d> trajectory;
    std::atomic<bool> redrawTrajectory;
    std::mutex trajectory_lock;

    std::vector<core::Point2d> fallbackTrajectory;
    std::atomic<bool> redrawFallbackTrajectory;
    std::mutex fallbackTrajectoryLock;

    perception::VoxelGridProto* grid;
    std::atomic<bool> redrawVoxels;
    std::mutex voxelsLock;
};

void initializeVisualization(VisualizationState* visualization) {
    CHECK_NOTNULL(visualization);
    visualization->redrawGroundTruthStates = false;
    visualization->redrawWaypoints = false;
    visualization->redrawPath = false;
    visualization->redrawTrajectory = false;
    visualization->redrawFallbackTrajectory = false;
    visualization->redrawVoxels = false;
    visualization->grid = nullptr;
}

std::ostream& operator<<(std::ostream& stream, const glm::mat4& pos) {
    stream << std::endl
           << pos[0][0] << " " << pos[0][1] << " " << pos[0][2] << " " << pos[0][3] << std::endl
           << pos[1][0] << " " << pos[1][1] << " " << pos[1][2] << " " << pos[1][3] << std::endl
           << pos[2][0] << " " << pos[2][1] << " " << pos[2][2] << " " << pos[2][3] << std::endl
           << pos[3][0] << " " << pos[3][1] << " " << pos[3][2] << " " << pos[3][3] << std::endl;
    return stream;
}

namespace estimation {

void estimateOdometry(VisualizationState* visualization, std::shared_ptr<WheelOdometryEstimator> estimator) {
    zmq::context_t context(1);
    constexpr int kDefaultPort = 7001;
    std::string address = "tcp://localhost:" + std::to_string(kDefaultPort);
    net::ZMQProtobufSubscriber<hal::VCUTelemetryEnvelope> subscriber(context, address, "telemetry", 1);

    while (s_active) {
        if (subscriber.poll()) {
            hal::VCUTelemetryEnvelope envelope;
            if (subscriber.recv(envelope)) {
                estimator->update(envelope);
                auto state = estimator->states().front();
                {
                    std::lock_guard<std::mutex> lock(visualization->odometryStatesLock);
                    visualization->odometryStates.emplace_front(state);
                }
            }
        }
    }
}

void estimateGroundTruth(VisualizationState* visualization, std::shared_ptr<Estimator> estimator) {
    CHECK_NOTNULL(visualization);

    bool init = false;
    estimation::State initial;
    initialiseState(initial);

    while (s_active) {
        auto states = estimator->states();
        if (states.empty()) {
            continue;
        }
        auto state = states.front();
        if (!init) {
            init = true;
            initial = state;
        }
        // Relative state
        state.m_pose = initial.m_pose.inverse() * state.m_pose;

        std::lock_guard<std::mutex> lock(visualization->groundTruthStatesLock);
        visualization->groundTruthStates.emplace_front(state);
        visualization->redrawGroundTruthStates = true;
        if (visualization->groundTruthStates.size() >= 100) {
            visualization->groundTruthStates.pop_back();
        }
    }
}

} // estimation

namespace perception {
class DefaultPerception : public Perception {
public:
    DefaultPerception(const PerceptionOptions& options)
        : Perception(options, std::unique_ptr<TerrainRepresentation>()) {}
};

void visualizePerception(VisualizationState* visualization, std::shared_ptr<estimation::Estimator> estimator,
    perception::visualization::Voxels* voxelsVisualizer) {
    using perception::VoxelGrid;
    using perception::VoxelGridOptions;
    using perception::Point3f;
    using perception::PointCloudXYZ;
    using perception::visualization::Grid;
    using perception::visualization::Voxels;

    CHECK_NOTNULL(visualization);

    constexpr float half_grid = 5;
    VoxelGridOptions options;
    options.mutable_grid_options()->set_dim_x(2 * half_grid);
    options.mutable_grid_options()->set_dim_y(2 * half_grid);
    options.mutable_grid_options()->set_dim_z(2 * half_grid);
    options.mutable_grid_options()->set_res_x(.5);
    options.mutable_grid_options()->set_res_y(.5);
    options.mutable_grid_options()->set_res_z(.5);

    VoxelGrid grid(options);
    VoxelGridProto serialized;
    visualization->grid = &serialized;

    bool init = false;
    Sophus::SE3d base;

    while (s_active) {
        for (float i = -half_grid; i < half_grid; i += 0.1f) {
            for (float j = -half_grid; j < half_grid; j += 0.1f) {
                for (float k = -half_grid; k < half_grid; k += 0.1f) {
                    PointCloudXYZ cloud;
                    cloud.xyz = Eigen::MatrixXf(3, 1);
                    cloud.xyz.col(0) = Point3f({ i, j, k });
                    CHECK(grid.add(cloud));
                }
            }
        }
        grid.computeSurflets();
        perception::serialize(grid, serialized);

        auto states = estimator->states();
        std::lock_guard<std::mutex> lock(visualization->voxelsLock);
        if (!states.empty()) {
            if (!init) {
                base = states.front().m_pose;
                init = true;
            }
            Sophus::SE3d openglFrame;
            planning::zippyToOpenGL(base.inverse() * states.front().m_pose, openglFrame);
            voxelsVisualizer->update(openglFrame);
        }
        visualization->redrawVoxels = true;
    }
}

} // perception

namespace planning {
namespace visualization {

    using perception::visualization::gl_point_t;

    class PathVisualizer : public perception::visualization::Primitive {
    public:
        PathVisualizer()
            : Primitive(
                  "packages/perception/bin/shaders/identity.vertexshader", "packages/perception/bin/shaders/identity.fragmentshader") {}

        void update(const std::vector<core::Point2d>& points) {
            CHECK(!points.empty());
            m_points.clear();
            for (size_t i = 1; i < points.size(); ++i) {
                auto previous = points[i - 1];
                m_points.push_back({ static_cast<float>(previous.x()), static_cast<float>(previous.y()), 0.0 });
                auto current = points[i];
                m_points.push_back({ static_cast<float>(current.x()), static_cast<float>(current.y()), 0.0 });
            }
        }

        void draw() override {
            if (m_points.empty()) {
                return;
            }

            glUseProgram(m_programID);
            checkGLErrorState();
            glEnable(GL_BLEND);
            glEnable(GL_LINE_SMOOTH);
            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
            glColor4f(0.0, 0.0, 1.0, 0.5);
            glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
            checkGLErrorState();
            glEnableClientState(GL_VERTEX_ARRAY);
            checkGLErrorState();
            glVertexPointer(3, GL_FLOAT, sizeof(gl_point_t), m_points.data());
            checkGLErrorState();
            glDrawArrays(GL_LINES, 0, m_points.size());
            checkGLErrorState();
            glDisableClientState(GL_VERTEX_ARRAY);
            checkGLErrorState();
            glDisable(GL_BLEND);
            glDisable(GL_LINE_SMOOTH);
            glUseProgram(0);
        }

    private:
        std::vector<gl_point_t> m_points;
    };

    class WaypointVisualizer : public perception::visualization::Primitive {
    public:
        WaypointVisualizer()
            : Primitive(
                  "packages/perception/bin/shaders/identity.vertexshader", "packages/perception/bin/shaders/identity.fragmentshader") {
            float theta = 0;
            while (theta <= static_cast<float>(2 * M_PI)) {
                constexpr float kRadius = kGoalDecisionBoundary;
                float x = kRadius * std::cos(theta);
                float y = kRadius * std::sin(theta);
                m_template.push_back({ x, y, 0 });
                constexpr float kDelta = 0.1f;
                theta += kDelta;
            }
        }

        void update(const std::vector<core::Point2d>& points) {
            CHECK(!points.empty());
            CHECK(!m_template.empty());
            m_points = m_template;
            const auto waypoint = points.back();
            std::for_each(m_points.begin(), m_points.end(), [waypoint](gl_point_t& point) {
                point.x += static_cast<float>(waypoint.x());
                point.y += static_cast<float>(waypoint.y());
            });
        }

        void draw() override {
            glUseProgram(m_programID);
            checkGLErrorState();
            glEnable(GL_BLEND);
            glEnable(GL_LINE_SMOOTH);
            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
            glColor4f(0.0, 1.0, 1.0, 0.5);
            checkGLErrorState();
            glEnableClientState(GL_VERTEX_ARRAY);
            checkGLErrorState();
            glVertexPointer(3, GL_FLOAT, sizeof(gl_point_t), m_points.data());
            checkGLErrorState();
            glDrawArrays(GL_LINES, 0, m_points.size());
            checkGLErrorState();
            glDisableClientState(GL_VERTEX_ARRAY);
            checkGLErrorState();
            glUseProgram(0);
        }

    private:
        std::vector<gl_point_t> m_template;
        std::vector<gl_point_t> m_points;
    };

    class TrajectoryVisualization : public perception::visualization::Primitive {
    public:
        TrajectoryVisualization()
            : Primitive(
                  "packages/perception/bin/shaders/identity.vertexshader", "packages/perception/bin/shaders/identity.fragmentshader") {}

        void update(const std::vector<core::Point2d>& trajectory) {
            CHECK(!trajectory.empty());
            m_points.clear();
            m_points.reserve(trajectory.size());
            for (const auto& point : trajectory) {
                m_points.push_back({ static_cast<float>(point.x()), static_cast<float>(point.y()), 0.0 });
            }
            CHECK(trajectory.size() == m_points.size());
        }

        void draw() override {
            glUseProgram(m_programID);
            checkGLErrorState();
            glEnable(GL_BLEND);
            glEnable(GL_POINT_SMOOTH);
            glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
            glColor4f(1.0, 0.0, 0.0, 0.2);
            glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
            checkGLErrorState();
            glEnableClientState(GL_VERTEX_ARRAY);
            checkGLErrorState();
            glVertexPointer(3, GL_FLOAT, sizeof(gl_point_t), m_points.data());
            checkGLErrorState();
            glDrawArrays(GL_POINTS, 0, m_points.size());
            checkGLErrorState();
            glDisableClientState(GL_VERTEX_ARRAY);
            checkGLErrorState();
            glUseProgram(0);
        }

    private:
        std::vector<gl_point_t> m_points;
    };

    class GoalGenerator : public perception::visualization::Primitive {
    public:
        GoalGenerator()
            : Primitive(
                  "packages/perception/bin/shaders/identity.vertexshader", "packages/perception/bin/shaders/identity.fragmentshader") {
            float theta = 0;
            while (theta <= static_cast<float>(2 * M_PI)) {
                constexpr float kRadius = 0.1;
                constexpr float kDelta = 0.05;
                float x = kRadius * std::cos(theta);
                float y = kRadius * std::sin(theta);
                m_pipper.push_back({ x, y, 0 });
                theta += kDelta;
            }
        }

        void update(const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj) {
            m_modelMatrix = glm::dmat4(model);
            m_viewMatrix = glm::dmat4(view);
            m_projectionMatrix = glm::dmat4(proj);
        }

        void draw() override {
            glUseProgram(m_programID);
            checkGLErrorState();
            glEnable(GL_BLEND);
            glEnable(GL_LINE_SMOOTH);
            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
            glColor4f(0.0, 1.0, 0.4, 0.5);
            checkGLErrorState();
            glEnableClientState(GL_VERTEX_ARRAY);
            checkGLErrorState();
            glVertexPointer(3, GL_FLOAT, sizeof(gl_point_t), m_pipper.data());
            checkGLErrorState();
            glDrawArrays(GL_LINES, 0, m_pipper.size());
            checkGLErrorState();
            glDisableClientState(GL_VERTEX_ARRAY);
            checkGLErrorState();
            glUseProgram(0);
        }

        void intersect() {
            SDL_GetMouseState(&m_mouseX, &m_mouseY);
            glGetIntegerv(GL_VIEWPORT, m_viewport);
            checkGLErrorState();
            auto realY = m_viewport[3] - (GLint)m_mouseY - 1;
            glReadPixels(m_mouseX, realY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &m_mouseZ);
            checkGLErrorState();
            CHECK(gluUnProject((GLdouble)m_mouseX, (GLdouble)realY, 1.0, glm::value_ptr(m_modelMatrix), glm::value_ptr(m_projectionMatrix),
                      m_viewport, &m_wx, &m_wy, &m_wz)
                == GLU_TRUE);

            glm::vec3 ray;
            ray[0] = m_wx;
            ray[1] = m_wy;
            ray[2] = m_wz;
            auto length = glm::length(ray);
            ray[0] /= length;
            ray[1] /= length;
            ray[2] /= length;

            LOG(INFO) << m_wx << ", " << m_wy << ", " << m_wz;
            m_transform.topRightCorner(3, 1) = Eigen::Vector3f{ static_cast<float>(m_wx), 0.0, static_cast<float>(m_wy) };
        }

    private:
        GLint m_viewport[4];
        glm::dmat4 m_modelMatrix, m_viewMatrix, m_projectionMatrix;
        int m_mouseX, m_mouseY, m_mouseZ;
        GLdouble m_wx, m_wy, m_wz;
        std::vector<gl_point_t> m_pipper;
    };

} // visualization
} // planning

void waypoint(VisualizationState* visualization, std::shared_ptr<estimation::Estimator> estimator) {
    CHECK_NOTNULL(visualization);
    CHECK_NOTNULL(estimator.get());
    using executor::ExecutorOptions;
    using executor::loadDefaultExecutorOptions;
    using perception::Perception;
    using perception::PerceptionOptions;
    using perception::DefaultPerception;
    using planning::ArcPlanner;
    using planning::ArcPlannerOptions;
    using planning::Comms;
    using planning::ZMQComms;
    using planning::loadDefaultArcPlannerOptions;
    using planning::loadDefaultTrajectoryOptions;
    using planning::TrajectoryPlanner;
    using planning::ConstrainedArcWrapper;
    using planning::ScaledArcWrapper;
    using planning::TrajectoryOptions;
    using planning::TrajectoryGenerator;
    using planning::SquareTrajectory;

    TrajectoryOptions trajectory_options = loadDefaultTrajectoryOptions();
    ExecutorOptions executor_options = loadDefaultExecutorOptions();
    ArcPlannerOptions planner_options = loadDefaultArcPlannerOptions();

    std::unique_ptr<TrajectoryGenerator> generator_ptr(new SquareTrajectory(trajectory_options));
    // Basic arc planner
    std::unique_ptr<ArcPlanner> core_trajectory_ptr(new ArcPlanner(planner_options, std::move(generator_ptr)));
    // Constraints: Ensure arc is valid
    std::unique_ptr<ArcPlanner> constraint_trajectory_ptr(new ConstrainedArcWrapper(std::move(core_trajectory_ptr)));
    // Constraints: Scale linear velocity wrt. arc curvature
    std::unique_ptr<ArcPlanner> trajectory_ptr(new ScaledArcWrapper(std::move(constraint_trajectory_ptr)));

    std::unique_ptr<Perception> perception_ptr(new DefaultPerception(PerceptionOptions()));
    std::unique_ptr<Comms> comms_ptr(
        new ZMQComms(executor_options.address(), executor_options.port(), executor_options.require_acknowledge()));

    using planning::SBPLPlanner;
    using planning::SBPLPlannerOptions;
    SBPLPlannerOptions options;
    options.set_planner_type(SBPLPlannerOptions::ARASTAR);
    std::unique_ptr<SBPLPlanner> path_ptr;
    path_ptr.reset(new SBPLPlanner(options));

    using planning::GoalDecisionFunction;
    using planning::EuclideanDistanceDecisionFunction;
    std::unique_ptr<GoalDecisionFunction> decision_ptr;
    decision_ptr.reset(new EuclideanDistanceDecisionFunction(kGoalDecisionBoundary));

    using planning::NavigatorOptions;
    NavigatorOptions navigator_options;
    navigator_options.set_frequency(10);

    std::unique_ptr<planning::WaypointNavigator> navigator_ptr;
    navigator_ptr.reset(new planning::WaypointNavigator(
        navigator_options, std::move(comms_ptr), std::move(trajectory_ptr), std::move(path_ptr), estimator, std::move(decision_ptr)));

    Sophus::SE3d target;
    target.translation()[planning::kXAxis] = 3;
    target.translation()[planning::kYAxis] = -1;

    navigator_ptr->addWaypointCallback(
        [visualization](const planning::PathElement& element, __attribute__((unused)) const Sophus::SE3d& baseFrame) {
            {
                // Set "current" waypoint
                std::lock_guard<std::mutex> guard(visualization->waypoints_lock);
                core::Point2d input, output;
                input.set_x(element.transform().translationx());
                input.set_y(element.transform().translationy());
                planning::zippyToOpenGL(input, output);
                visualization->waypoints.push_back(output);
                visualization->redrawWaypoints = true;
            }
        });

    navigator_ptr->addPathCallback([visualization](const planning::Path& path, __attribute__((unused)) const estimation::State& state) {
        {
            std::vector<core::Point2d> points;
            for (int i = 0; i < path.elements_size(); ++i) {
                core::Point2d input, output;
                input.set_x(path.elements(i).transform().translationx());
                input.set_y(path.elements(i).transform().translationy());
                planning::zippyToOpenGL(input, output);
                points.push_back(output);
            }
            std::lock_guard<std::mutex> guard(visualization->pathLock);
            visualization->path.swap(points);
            visualization->redrawPath = true;
        }
    });

    navigator_ptr->addTrajectoryCallback([visualization](const planning::Trajectory& trajectory, const estimation::State& state) {
        {
            constexpr float kSampleDistance = 0.05;
            std::vector<core::Point2d> sampled;
            CHECK(sample(trajectory.elements(0), sampled, kSampleDistance));
            std::vector<core::Point2d> points;
            [sampled, &points, state]() {
                for (const auto& point : sampled) {
                    Eigen::Vector4d pt{ point.x(), point.y(), 0.0, 1.0 };
                    Eigen::Vector4d result = state.m_pose.matrix() * pt;
                    Eigen::Vector4d output;
                    planning::zippyToOpenGL(result, output);
                    core::Point2d global_frame_pt;
                    global_frame_pt.set_x(output[planning::kXAxis]);
                    global_frame_pt.set_y(output[planning::kYAxis]);
                    points.push_back(global_frame_pt);
                }
            }();
            CHECK(sampled.size() == points.size());
            std::lock_guard<std::mutex> guard(visualization->trajectory_lock);
            visualization->trajectory = points;
            visualization->redrawTrajectory = true;
        }
    });

    navigator_ptr->addFallbackTrajectoryCallback(
        [visualization](const planning::Trajectory& trajectory, const estimation::State& state, const planning::Path& remainingWaypoints) {
            {
                std::vector<core::Point2d> aggregated;

                for (int i = 0; i < trajectory.elements_size(); ++i) {
                    std::vector<core::Point2d> sampled;
                    constexpr float kSampleDistance = 0.05;
                    CHECK(sample(trajectory.elements(i), sampled, kSampleDistance));
                    Sophus::SE3d intermediate_pose(Eigen::Quaterniond{ 1, 0, 0, 0 }, Eigen::Vector3d{ 0, 0, 0 });
                    if (i > 0) {
                        planning::protoToPose(remainingWaypoints.elements(i - 1).transform(), intermediate_pose);
                    }
                    std::vector<core::Point2d> points;
                    [sampled, &points, state, intermediate_pose]() {
                        for (const auto& point : sampled) {
                            Eigen::Vector4d pt{ point.x(), point.y(), 0.0, 1.0 };
                            Eigen::Vector4d result = state.m_pose.matrix() * intermediate_pose.matrix() * pt;
                            Eigen::Vector4d output;
                            planning::zippyToOpenGL(result, output);
                            core::Point2d global_frame_pt;
                            global_frame_pt.set_x(output[planning::kXAxis]);
                            global_frame_pt.set_y(output[planning::kYAxis]);
                            points.push_back(global_frame_pt);
                        }
                    }();
                    std::copy(points.begin(), points.end(), std::back_inserter(aggregated));
                }
                std::lock_guard<std::mutex> guard(visualization->fallbackTrajectoryLock);
                visualization->fallbackTrajectory = aggregated;
                visualization->redrawFallbackTrajectory = true;
            }
        });

    while (s_active) {
        auto future = navigator_ptr->navigateTo(target);
        CHECK(future.get() == true);
        target.translation()[planning::kYAxis] *= -1;
    }
}

void updateVisualization(VisualizationState* visualization, planning::visualization::TrajectoryVisualization* trajectory,
    planning::visualization::TrajectoryVisualization* fallbackTrajectory, estimation::visualization::PoseVisualizer* groundTruthPose,
    estimation::visualization::PoseVisualizer* odometryPose, planning::visualization::WaypointVisualizer* waypoints,
    planning::visualization::PathVisualizer* path, perception::visualization::Voxels* voxels) {
    CHECK_NOTNULL(visualization);
    CHECK_NOTNULL(trajectory);
    CHECK_NOTNULL(fallbackTrajectory);
    CHECK_NOTNULL(groundTruthPose);
    CHECK_NOTNULL(waypoints);
    CHECK_NOTNULL(path);
    CHECK_NOTNULL(voxels);
    if (visualization->redrawGroundTruthStates) {
        if (visualization->groundTruthStatesLock.try_lock()) {
            groundTruthPose->updateStates(visualization->groundTruthStates);
            visualization->redrawGroundTruthStates = false;
            visualization->groundTruthStatesLock.unlock();
        }
    }
    if (visualization->redrawOdometryStates) {
        if (visualization->odometryStatesLock.try_lock()) {
            odometryPose->updateStates(visualization->odometryStates);
            visualization->redrawOdometryStates = false;
            visualization->odometryStatesLock.unlock();
        }
    }
    if (visualization->redrawWaypoints) {
        if (visualization->waypoints_lock.try_lock()) {
            waypoints->update(visualization->waypoints);
            visualization->redrawWaypoints = false;
            visualization->waypoints_lock.unlock();
        }
    }
    if (visualization->redrawPath) {
        if (visualization->pathLock.try_lock()) {
            path->update(visualization->path);
            visualization->redrawPath = false;
            visualization->pathLock.unlock();
        }
    }
    if (visualization->redrawTrajectory) {
        if (visualization->trajectory_lock.try_lock()) {
            trajectory->update(visualization->trajectory);
            visualization->redrawTrajectory = false;
            visualization->trajectory_lock.unlock();
        }
    }
    if (visualization->redrawFallbackTrajectory) {
        if (visualization->fallbackTrajectoryLock.try_lock()) {
            fallbackTrajectory->update(visualization->fallbackTrajectory);
            visualization->redrawFallbackTrajectory = false;
            visualization->fallbackTrajectoryLock.unlock();
        }
    }
    if (visualization->redrawVoxels) {
        if (visualization->voxelsLock.try_lock()) {
            CHECK_NOTNULL(visualization->grid);
            voxels->update(*visualization->grid);
            visualization->redrawVoxels = false;
            visualization->voxelsLock.unlock();
        }
    }
}

int main(int argc, char** argv) {
    gflags::SetUsageMessage("Navigation visualizer");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, &argv, false);

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

    using perception::visualization::Grid;

    glm::mat4 projection = glm::perspectiveFov(70.0f, static_cast<float>(width), static_cast<float>(height), 0.1f, 100.0f);

    using perception::visualization::Camera;
    using perception::visualization::Interactor;
    Camera camera;
    camera.reset();
    Interactor interactor;
    interactor.setCamera(&camera);
    interactor.setScreenSize(width, height);

    std::vector<perception::visualization::Primitive*> primitives;

    // Grid : 1m
    Grid grid;
    primitives.push_back(&grid);

    // Random goal generator for planner
    planning::visualization::GoalGenerator generator;
    primitives.push_back(&generator);

    // Visualize the immediate trajectory
    planning::visualization::TrajectoryVisualization trajectoryVisualizer;
    primitives.push_back(&trajectoryVisualizer);

    // Visualize the fallback trajectory
    planning::visualization::TrajectoryVisualization fallbackTrajectoryVisualizer;
    primitives.push_back(&fallbackTrajectoryVisualizer);

    // Visualize waypoints
    planning::visualization::WaypointVisualizer waypointVisualizer;
    primitives.push_back(&waypointVisualizer);

    // Visualize global path
    planning::visualization::PathVisualizer pathVisualizer;
    primitives.push_back(&pathVisualizer);

    // Visualize pose: Ground truth
    estimation::visualization::PoseVisualizer groundTruthVisualizer;
    primitives.push_back(&groundTruthVisualizer);
    // Visualize pose: Odometry
    estimation::visualization::PoseVisualizer odometryVisualizer;
    primitives.push_back(&odometryVisualizer);

    using perception::visualization::Voxels;
    // Voxels
    Voxels voxelVisualizer;
    primitives.push_back(&voxelVisualizer);

    // Common visualization data structure
    VisualizationState visualization;
    initializeVisualization(&visualization);

    using estimation::GroundTruthEstimator;
    using estimation::GroundTruthEstimatorOptions;
    GroundTruthEstimatorOptions groundTruthEstimatorOptions;
    groundTruthEstimatorOptions.mutable_base_options()->set_subscribeaddress("tcp://localhost");
    groundTruthEstimatorOptions.mutable_base_options()->set_subscribeport(7501);
    groundTruthEstimatorOptions.mutable_base_options()->set_publishaddress("tcp://127.0.0.1");
    groundTruthEstimatorOptions.mutable_base_options()->set_publishport(7101);
    std::shared_ptr<GroundTruthEstimator> groundTruthEstimator;
    groundTruthEstimator.reset(new GroundTruthEstimator(groundTruthEstimatorOptions));

    using estimation::WheelOdometryEstimator;
    using estimation::WheelOdometryOptions;
    using estimation::loadDefaultWheelOdometryOptions;
    WheelOdometryOptions wheelOdometryOptions = loadDefaultWheelOdometryOptions();

    std::shared_ptr<WheelOdometryEstimator> wheelOdometryEstimator;
    wheelOdometryEstimator.reset(new WheelOdometryEstimator(wheelOdometryOptions));

    std::vector<std::thread> threads;
    threads.emplace_back(std::thread(estimation::estimateGroundTruth, &visualization, groundTruthEstimator));
    threads.emplace_back(std::thread(estimation::estimateOdometry, &visualization, wheelOdometryEstimator));
    threads.emplace_back(std::thread(perception::visualizePerception, &visualization, groundTruthEstimator, &voxelVisualizer));

    if ("waypoint" == FLAGS_mode) {
        if ("groundTruth" == FLAGS_pose) {
            threads.emplace_back(std::thread(&waypoint, &visualization, groundTruthEstimator));
        } else if ("odometry" == FLAGS_pose) {
            threads.emplace_back(std::thread(&waypoint, &visualization, wheelOdometryEstimator));
        }
    } else if (FLAGS_mode == "point_and_go") {
        throw std::runtime_error("Currently unimplemented");
    } else {
        throw std::runtime_error("Unsupported mode!");
    }

    bool primary_pressed = false;
    SDL_Event event;
    do {
        updateVisualization(&visualization, &trajectoryVisualizer, &fallbackTrajectoryVisualizer, &groundTruthVisualizer,
            &odometryVisualizer, &waypointVisualizer, &pathVisualizer, &voxelVisualizer);

        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                s_active = false;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    primary_pressed = true;
                }
                if (event.button.button == SDL_BUTTON_RIGHT) {
                    generator.intersect();
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

        auto model = glm::mat4();
        auto view = camera.getMatrix();
        auto MVP = projection * view * model;

        interactor.update();
        generator.update(model, view, projection);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto* primitive : primitives) {
            primitive->update(MVP);
            checkGLErrorState();
            primitive->draw();
            checkGLErrorState();
        }
        SDL_GL_SwapWindow(displayWindow);

    } while (s_active);

    for (auto& thread : threads) {
        thread.join();
    }
    return EXIT_SUCCESS;
}
