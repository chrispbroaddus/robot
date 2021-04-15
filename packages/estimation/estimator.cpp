#include "packages/estimation/estimator.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <mutex>
#include <thread>

#include "Eigen/Geometry"
#include "glog/logging.h"
#include "google/protobuf/text_format.h"
#include "sophus/se3.hpp"

#include "packages/estimation/thirdparty/eigen_utils.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/unity_plugins/proto/unity_telemetry_envelope.pb.h"

namespace estimation {

void initialiseState(State& state) {
    state.m_time = std::chrono::nanoseconds(0);
    state.m_pose = Sophus::SE3d(Eigen::Quaterniond{ 1, 0, 0, 0 }, Eigen::Vector3d{ 0, 0, 0 });
    state.m_cov.setZero();
    state.m_v.setZero();
    state.m_w.setZero();
}

bool Estimator::get(const core::SystemTimestamp& timestamp, State& state) {
    State query_state;
    query_state.m_time = std::chrono::nanoseconds(timestamp.nanos());

    if (m_stateHistory.empty()) {
        // No valid states in the buffer
        return false;
    }

    auto it = std::lower_bound(m_stateHistory.begin(), m_stateHistory.end(), query_state,
        [](const State& lhs, const State& rhs) { return lhs.m_time < rhs.m_time; });

    if (it == m_stateHistory.end()) {
        // The specified search timestamp is after the last state of the buffer,
        // but it might be within the search window tolerance.
        it = std::prev(it);
    }
    auto nanoseconds = std::chrono::nanoseconds(timestamp.nanos());
    if (std::abs((it->m_time - nanoseconds).count()) > m_defaultSearchWindow.count()) {
        // No valid states within the specified window in the buffer
        return false;
    }
    state = *it;
    return true;
}

OdometryEstimator::OdometryEstimator(const EstimatorOptions& options)
    : m_active(true)
    , m_options(options) {
    m_linearVelocity.setZero();
    m_rotationalVelocity.setZero();
    m_frequency = m_options.frequency();
    auto fullyQualifiedAddress = m_options.publishaddress() + ":" + std::to_string(m_options.publishport());
    LOG(INFO) << "Publishing to: " << fullyQualifiedAddress;
    m_publisher.reset(new net::ZMQProtobufPublisher<estimation::StateProto>(m_context, fullyQualifiedAddress, 1, 100));
    m_executionThread = std::thread(&OdometryEstimator::run, this);
}

OdometryEstimator::~OdometryEstimator() {
    m_active = false;
    if (m_executionThread.joinable()) {
        m_executionThread.join();
    }
}

void OdometryEstimator::run() {
    CHECK(m_frequency > 0);

    using core::chrono::gps::wallClockInNanoseconds;
    const auto period = std::chrono::milliseconds(static_cast<int>(1 / m_frequency * 1000));

    hal::Device smoothFrameOriginAnchor;
    smoothFrameOriginAnchor.set_name("smoothFrameOrigin");
    calibration::CoordinateFrame smoothFrameOrigin;
    *smoothFrameOrigin.mutable_device() = smoothFrameOriginAnchor;

    hal::Device vehicleFrameAnchor;
    vehicleFrameAnchor.set_name("vehicleFrame");
    calibration::CoordinateFrame vehicleFrame;
    *vehicleFrame.mutable_device() = vehicleFrameAnchor;

    calibration::CoordinateTransformation transform;
    *transform.mutable_sourcecoordinateframe() = smoothFrameOrigin;
    *transform.mutable_targetcoordinateframe() = vehicleFrame;

    auto lastUpdate = wallClockInNanoseconds();

    while (m_active) {
        auto update = wallClockInNanoseconds();
        auto nextUpdateTime = update + period;
        this->update(update - lastUpdate);
        auto states = this->states();
        if (!states.empty()) {
            using planning::poseToProto;
            const auto& estimatorState = states.front();
            poseToProto(estimatorState.m_pose, transform);
            estimation::StateProto state;
            // Pose
            *state.mutable_transform() = transform;
            // Velocities
            state.set_v_x(estimatorState.m_v[0]);
            state.set_v_y(estimatorState.m_v[1]);
            state.set_v_z(estimatorState.m_v[2]);
            state.set_w_x(estimatorState.m_w[0]);
            state.set_w_y(estimatorState.m_w[1]);
            state.set_w_z(estimatorState.m_w[2]);
            state.mutable_system_timestamp()->set_nanos(estimatorState.m_time.count());
            state.mutable_hardware_timestamp()->set_nanos(estimatorState.m_time.count());

            m_publisher->send(state, "odometry");
            if (m_options.verbosity() > 0) {
                LOG(INFO) << state.DebugString();
            }
        }
        lastUpdate = update;
        auto now = wallClockInNanoseconds();
        auto sleepDuration = (update + period) - now;
        if (sleepDuration.count() > 0) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(sleepDuration));
        }
    }
}

void OdometryEstimator::propagateState(const std::chrono::nanoseconds& time) {
    Eigen::Vector3d linearVelocity;
    Eigen::Vector3d rotationalVelocity;
    {
        std::lock_guard<std::mutex> lock(m_velocityGuard);
        linearVelocity = m_linearVelocity;
        rotationalVelocity = m_rotationalVelocity;
    }
    const Eigen::Matrix4d omega_current = Omega(rotationalVelocity);

    if (m_stateHistory.empty()) {
        State initialState;
        initialiseState(initialState);
        m_stateHistory.emplace_front(initialState);
    }
    State currentState = m_stateHistory.front();

    const Eigen::Matrix4d omega_previous = Omega(currentState.m_w);
    Eigen::Matrix4d omega_mean = Omega((rotationalVelocity + currentState.m_w) / 2.0);
    const double dt = static_cast<double>(time.count() / 1e9);
    CHECK(dt > 0);
    int div = 1;
    Eigen::Matrix4d matExp;
    matExp.setIdentity();
    omega_mean *= .5 * dt;

    constexpr int kDefaultNumIntegrationSteps = 5;
    for (int i = 1; i < kDefaultNumIntegrationSteps; ++i) {
        div *= i;
        matExp = matExp + omega_mean / div;
        omega_mean *= omega_mean;
    }
    const Eigen::Matrix4d quaternion_integration
        = matExp + 1.0 / 48.0 * (omega_current * omega_previous - omega_previous * omega_current) * dt * dt;
    Eigen::Quaterniond rotation = currentState.m_pose.unit_quaternion();
    rotation.coeffs() = quaternion_integration * rotation.coeffs();
    rotation.normalize();

    Eigen::Vector3d world_frame_velocity = rotation.toRotationMatrix() * linearVelocity;
    Eigen::Vector3d previous_position = currentState.m_pose.translation();

    Eigen::Vector3d current_position = previous_position + (((world_frame_velocity + currentState.m_v) / 2) * dt);
    currentState.m_pose = Sophus::SE3d(rotation, current_position);
    currentState.m_v = world_frame_velocity;
    currentState.m_w = rotation.toRotationMatrix() * rotationalVelocity;

    m_stateHistory.emplace_front(currentState);
}

void OdometryEstimator::propagateUncertainty(__attribute__((unused)) const std::chrono::nanoseconds& time) {
    throw std::runtime_error("Not yet implemented: AUTO-269");
}

void OdometryEstimator::update(const std::chrono::nanoseconds& time) {
    propagateState(time);
    if (m_options.propagateuncertainty()) {
        propagateUncertainty(time);
    }
}

bool isValid(const double linearVelocity, const double rotationalVelocity, const double elapsedTimeInSeconds) {
    constexpr double kMaxInstantaneousLinearVelocityThreshold = 5.0;
    constexpr double kMaxInstantaneousRotatationalVelocityThreshold = 5.0;
    constexpr double kMaxIntervalInSeconds = 0.2;
    if (std::abs(linearVelocity) >= kMaxInstantaneousLinearVelocityThreshold) {
        return false;
    }
    if (std::abs(rotationalVelocity) >= kMaxInstantaneousRotatationalVelocityThreshold) {
        return false;
    }
    if (elapsedTimeInSeconds <= 0 || elapsedTimeInSeconds >= kMaxIntervalInSeconds) {
        return false;
    }
    return true;
}

class WheelOdometryEstimator::WheelOdometryImpl {
public:
    WheelOdometryImpl(const WheelOdometryOptions& options)
        : m_options(options) {}

    void update(const hal::VCUTelemetryEnvelope& telemetry) {
        if (telemetry.has_servo()) {
            updateSteering(telemetry);
        } else if (telemetry.has_wheelencoder()) {
            updateVelocities(telemetry);
        }
    }

    void updateVelocities(const hal::VCUTelemetryEnvelope& telemetry) {
        const auto id = telemetry.wheelencoder().wheelid();
        constexpr hal::VCUWheelID kDefaultWheelID = hal::VCUWheelID::LeftFrontWheel;
        if (id == kDefaultWheelID) {
            const double displacement = telemetry.wheelencoder().lineardisplacementmeters();
            const auto duration = std::chrono::nanoseconds(telemetry.wheelencoder().periodendhardwaretimestamp().nanos()
                - telemetry.wheelencoder().periodstarthardwaretimestamp().nanos());

            if (duration.count() <= 0) {
                LOG(ERROR) << telemetry.DebugString();
                throw std::runtime_error("Invalid time period!");
            }
            const auto elapsedTimeInSeconds = std::chrono::duration<double>(duration).count();

            CHECK(elapsedTimeInSeconds > 0);
            const auto wheelVelocity = displacement / elapsedTimeInSeconds;

            CHECK(!std::isnan(m_steerAngle)) << telemetry.DebugString();
            CHECK(!std::isinf(m_steerAngle)) << telemetry.DebugString();
            const double linearVelocity = wheelVelocity * std::cos(m_steerAngle);

            CHECK(m_options.distancetosentinelwheel() > 0);
            const double rotationalComponent = wheelVelocity * std::sin(m_steerAngle);
            const double sentinelTravelCircumference = static_cast<float>(2 * M_PI) * m_options.distancetosentinelwheel();

            double rotationalVelocity = 0;
            if (std::abs(rotationalComponent) > std::numeric_limits<double>::epsilon()) {
                auto timeForOneRotation = (sentinelTravelCircumference / rotationalComponent);
                rotationalVelocity = (2 * M_PI / timeForOneRotation);
            }

            if (m_options.base_options().verbosity() > 1) {
                LOG(INFO) << "Elapsed (ms):     " << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
                LOG(INFO) << "Steer (rad):      " << std::fixed << std::showpos << std::setw(5) << m_steerAngle;
                LOG(INFO) << "Displacement (m): " << std::fixed << std::showpos << std::setw(5) << std::setprecision(6) << displacement;
                LOG(INFO) << "v (m/s), (rad/s): " << std::fixed << std::showpos << std::setw(5) << std::setprecision(6) << m_linearVelocity
                          << " : " << m_rotationalVelocity;
                LOG(INFO) << "-------------";
            }
            if (!isValid(linearVelocity, rotationalVelocity, elapsedTimeInSeconds)) {
                LOG(ERROR) << "Invalid VCU message received";
                ++m_invalidMessages;
                return;
            }
            CHECK(!std::isinf(linearVelocity)) << telemetry.DebugString();
            CHECK(!std::isnan(linearVelocity)) << telemetry.DebugString();
            CHECK(!std::isinf(rotationalVelocity)) << telemetry.DebugString();
            CHECK(!std::isnan(rotationalVelocity)) << telemetry.DebugString();
            LOG_IF(WARNING, std::abs(rotationalVelocity) > 4) << rotationalVelocity << " Huge rotational velocity component";
            LOG_IF(WARNING, std::abs(linearVelocity) > 5) << linearVelocity << " Huge linear velocity component";
            m_linearVelocity = linearVelocity;
            m_rotationalVelocity = rotationalVelocity;
        }
    }

    void updateSteering(const hal::VCUTelemetryEnvelope& telemetry) {
        constexpr hal::VCUServoID kDefaultServoID = hal::VCUServoID::LeftFrontServo;
        if (telemetry.servo().servoid() == kDefaultServoID) {
            m_steerAngle = telemetry.servo().servoangleradians();
        }
    }

    double linearVelocity() const { return m_linearVelocity; }
    double rotationalVelocity() const { return m_rotationalVelocity; }

private:
    const WheelOdometryOptions m_options;
    State m_currentState;
    double m_steerAngle = 0.0;
    double m_linearVelocity = 0.0;
    double m_rotationalVelocity = 0.0;

    int m_invalidMessages = 0;
};

WheelOdometryEstimator::WheelOdometryEstimator(const WheelOdometryOptions& options)
    : OdometryEstimator(options.base_options())
    , m_impl(new WheelOdometryImpl(options)) {}

void WheelOdometryEstimator::update(const hal::VCUTelemetryEnvelope& telemetry) {
    m_impl->update(telemetry);
    Eigen::Vector3d linearVelocity{ m_impl->linearVelocity(), 0.0, 0.0 };
    Eigen::Vector3d rotationalVelocity{ 0.0, 0.0, m_impl->rotationalVelocity() };
    {
        std::lock_guard<std::mutex> lock(m_velocityGuard);
        m_linearVelocity = linearVelocity;
        m_rotationalVelocity = rotationalVelocity;
    }
}

void unityTelemetryToState(const unity_plugins::UnityTelemetryEnvelope& envelope, State& state) {
    state.m_time = std::chrono::nanoseconds(0);
    Eigen::Vector3d position{ envelope.vehiclepose().transformations().translationx(),
        envelope.vehiclepose().transformations().translationy(), envelope.vehiclepose().transformations().translationz() };
    Eigen::Vector3d axis{ envelope.vehiclepose().transformations().rodriguesrotationx(),
        envelope.vehiclepose().transformations().rodriguesrotationy(), envelope.vehiclepose().transformations().rodriguesrotationz() };
    auto angle = axis.norm();
    axis /= angle;
    Eigen::AngleAxisd rotation{ angle, axis };
    state.m_pose = Sophus::SE3d(Eigen::Quaterniond(rotation), position);
}

class GroundTruthEstimator::GroundTruthEstimatorImpl {
public:
    GroundTruthEstimatorImpl(const GroundTruthEstimatorOptions& options)
        : m_active(true) {
        m_executionThread = std::thread(&GroundTruthEstimatorImpl::run, this, options);
    }

    ~GroundTruthEstimatorImpl() {
        m_active = false;
        if (m_executionThread.joinable()) {
            m_executionThread.join();
        }
    }

    void run(const GroundTruthEstimatorOptions& options) {
        auto fullyQualifiedSubscriberAddress
            = options.base_options().subscribeaddress() + ":" + std::to_string(options.base_options().subscribeport());
        auto fullyQualifiedPublisherAddress
            = options.base_options().publishaddress() + ":" + std::to_string(options.base_options().publishport());

        zmq::context_t context(1);
        net::ZMQProtobufSubscriber<unity_plugins::UnityTelemetryEnvelope> subscriber(context, fullyQualifiedSubscriberAddress, "", 1);
        m_publisher.reset(new net::ZMQProtobufPublisher<estimation::StateProto>(context, fullyQualifiedPublisherAddress, 1, 100));

        while (m_active) {
            if (subscriber.poll(std::chrono::milliseconds())) {
                unity_plugins::UnityTelemetryEnvelope envelope;
                if (!subscriber.recv(envelope)) {
                    continue;
                }
                if (envelope.has_vehiclepose()) {
                    State state;
                    unityTelemetryToState(envelope, state);
                    using planning::poseToProto;
                    calibration::CoordinateTransformation transform;
                    poseToProto(state.m_pose, transform);
                    estimation::StateProto stateProto;
                    *stateProto.mutable_transform() = transform;
                    m_publisher->send(stateProto, "ground_truth");
                    if (!m_stateHistory.empty()) {
                        auto distance = (m_stateHistory.front().m_pose.inverse() * state.m_pose).translation().norm();
                        constexpr double kDistanceThreshold = 0.2;
                        if (distance > kDistanceThreshold) {
                            LOG(FATAL) << "Pose delta violation!" << distance;
                        }
                    }
                    std::lock_guard<std::mutex> lock(m_stateGuard);
                    m_stateHistory.push_front(state);
                }
            }
        }
    }

    std::deque<State> states() {
        std::lock_guard<std::mutex> lock(m_stateGuard);
        return m_stateHistory;
    }

private:
    std::thread m_executionThread;
    std::mutex m_stateGuard;
    std::atomic<bool> m_active;
    std::deque<State> m_stateHistory;
    std::unique_ptr<net::ZMQProtobufPublisher<estimation::StateProto> > m_publisher;
};

GroundTruthEstimator::GroundTruthEstimator(const GroundTruthEstimatorOptions& options)
    : m_impl(new GroundTruthEstimatorImpl(options)) {}

std::deque<State> GroundTruthEstimator::states() { return m_impl->states(); }

} // estimation
