#pragma once

#include <atomic>
#include <chrono>
#include <deque>
#include <fstream>
#include <mutex>
#include <thread>

#include "Eigen/Geometry"
#include "glog/logging.h"
#include "google/protobuf/text_format.h"
#include "sophus/se3.hpp"

#include "packages/calibration/proto/coordinate_transformation.pb.h"
#include "packages/core/include/chrono.h"
#include "packages/core/proto/geometry.pb.h"
#include "packages/core/proto/timestamp.pb.h"
#include "packages/estimation/proto/estimator_options.pb.h"
#include "packages/estimation/proto/state.pb.h"
#include "packages/estimation/proto_helpers.h"
#include "packages/hal/proto/vcu_telemetry_envelope.pb.h"
#include "packages/net/include/zmq_topic_pub.h"
#include "packages/planning/utils.h"
#include "packages/unity_plugins/proto/ground_truth_vehicle_pose.pb.h"

namespace estimation {

typedef struct {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    std::chrono::nanoseconds m_time;
    Sophus::SE3<double> m_pose;
    Eigen::Matrix<double, 6, 6> m_cov;
    Eigen::Vector3d m_v;
    Eigen::Vector3d m_w;
} State;

void initialiseState(State& state);

class Estimator {
public:
    Estimator()
        : m_defaultSearchWindow(1) {}

    virtual ~Estimator() = default;

    bool get(const core::SystemTimestamp& timestamp, State& state);

    virtual std::deque<State> states() { return m_stateHistory; }

protected:
    std::chrono::nanoseconds m_defaultSearchWindow;
    std::deque<State> m_stateHistory;
};

/**
 * @brief A class conforming to the Estimator definitions, that reports the
 *        ground-truth position of the vehicle in the simultaor.
 */
class GroundTruthEstimator : public Estimator {
public:
    GroundTruthEstimator(const GroundTruthEstimatorOptions& options);

    std::deque<State> states() override;

private:
    class GroundTruthEstimatorImpl;
    std::shared_ptr<GroundTruthEstimatorImpl> m_impl;
};

/**
 * @brief Base-class for odometry-type estimators.
 */
class OdometryEstimator : public Estimator {
public:
    OdometryEstimator(const EstimatorOptions& options);

    virtual ~OdometryEstimator();

protected:
    void propagateState(const std::chrono::nanoseconds& time);
    void propagateUncertainty(const std::chrono::nanoseconds& time);
    void update(const std::chrono::nanoseconds& time);
    void run();

    std::mutex m_velocityGuard;
    Eigen::Vector3d m_linearVelocity;
    Eigen::Vector3d m_rotationalVelocity;

    std::atomic<bool> m_active;
    std::thread m_executionThread;

    float m_frequency;
    std::string m_address;
    zmq::context_t m_context = zmq::context_t(1);
    std::unique_ptr<net::ZMQProtobufPublisher<estimation::StateProto> > m_publisher;

private:
    const EstimatorOptions& m_options;
};

/**
 * @brief Estimator using the VCU telemetry (real, or simulated)
 */
class WheelOdometryEstimator : public OdometryEstimator {
public:
    WheelOdometryEstimator(const WheelOdometryOptions& options);

    void update(const hal::VCUTelemetryEnvelope& telemetry);

private:
    class WheelOdometryImpl;
    std::shared_ptr<WheelOdometryImpl> m_impl;
};

} // estimation
