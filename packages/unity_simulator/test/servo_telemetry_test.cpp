#include "glog/logging.h"
#include "gtest/gtest.h"

#include <atomic>
#include <cmath>
#include <thread>
#include <unordered_map>

#include "packages/core/include/chrono.h"
#include "packages/hal/proto/vcu_command_envelope.pb.h"
#include "packages/hal/proto/vcu_command_response.pb.h"
#include "packages/hal/proto/vcu_telemetry_envelope.pb.h"
#include "packages/hal/proto/vcu_wheel_encoder_telemetry.pb.h"
#include "packages/net/include/zmq_req_client.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/unity_plugins/connections_reader/include/connections_reader.h"

namespace {
constexpr auto DEFAULT_CONNECTIONS_FILE = "packages/unity_simulator/test/connections.default.json";
}

void sendTrajectoryCommand(
    zmq::context_t& context, const std::string& trajectorySubscriberAddress, const float velocity, const float curvature) {

    net::ZMQProtobufReqClient<hal::VCUCommandEnvelope, hal::VCUCommandResponse> client(context, trajectorySubscriberAddress, 1000, 1000);

    int counter = 1;

    hal::VCUTrajectoryCommand* trajectory = new hal::VCUTrajectoryCommand();

    constexpr uint64_t gpsTimestampOffsetInNanos = 1e6;
    auto gpsTimestamp = core::chrono::gps::wallClockInNanoseconds();

    uint32_t millisecondPerElement = 500;
    int numSegments = 20;
    for (int i = 0; i < numSegments; i++) {
        uint64_t futureTimestamp
            = gpsTimestamp.count() + (uint64_t)i * millisecondPerElement * (uint64_t)1000000 + gpsTimestampOffsetInNanos;
        auto segment = trajectory->add_segments();
        segment->mutable_targetstarttime()->set_nanos(futureTimestamp);
        hal::VCUArcDriveSegment arcDriveSegment;
        arcDriveSegment.set_linearvelocitymeterspersecond(velocity);
        arcDriveSegment.set_curvatureinversemeters(curvature);
        *segment->mutable_arcdrive() = arcDriveSegment;
    }
    trajectory->mutable_segments(numSegments - 1)->mutable_arcdrive()->set_linearvelocitymeterspersecond(0);

    fprintf(stdout, "Sending out the data!\n");

    hal::VCUCommandEnvelope envelope;
    envelope.set_sequencenumber(++counter);
    envelope.set_allocated_trajectorycommand(trajectory);
    bool sendSuccess = client.send(envelope);

    hal::VCUCommandResponse response;
    client.recv(response);
    LOG(INFO) << __PRETTY_FUNCTION__ << " ... VCUCommandResponse error reason : " << response.error_reason();

    CHECK(sendSuccess);
}

void sendTrajectoryCommandMovingRight(zmq::context_t& context, const std::string& trajectorySubscriberAddress) {
    sendTrajectoryCommand(context, trajectorySubscriberAddress, 1, 0.1);
}

void sendTrajectoryCommandMovingLeft(zmq::context_t& context, const std::string& trajectorySubscriberAddress) {
    sendTrajectoryCommand(context, trajectorySubscriberAddress, 1, -0.1);
}

///
/// @brief Checks the correctness of the signs of the servo telemetry
/// @details It checks the signs of the servo telemetry while the vehicle is moving.
///          When the curvature center is on the right outside the vehicle,
///          the front wheels should have positive angle, whereas the rear wheels have negative angles.
///          Opposite, when the curvature center is on the left side.
///
TEST(TelemetryDataValidation, checkWheelServoSignMovingRight) {

    unity_plugins::ConnectionsReader reader(DEFAULT_CONNECTIONS_FILE);
    const auto telemetryAddress = reader.fullAddress(mercury::topic::telemetry);
    const auto trajectorySubscriberAddress = reader.fullAddress(mercury::topic::trajectory);

    zmq::context_t context(1);
    sendTrajectoryCommandMovingRight(context, trajectorySubscriberAddress);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    net::ZMQProtobufSubscriber<hal::VCUTelemetryEnvelope> vcuTelemSub(context, telemetryAddress, "telemetry", 1);

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> fp_ms = t2 - t1;

    bool existServoData = false;
    while (fp_ms < std::chrono::seconds(5)) {

        if (vcuTelemSub.poll(std::chrono::milliseconds(10))) {
            hal::VCUTelemetryEnvelope vcuTelemEnvelope;
            vcuTelemSub.recv(vcuTelemEnvelope);

            if (vcuTelemEnvelope.has_servo()) {
                auto servo = vcuTelemEnvelope.servo();
                if (servo.servoid() == hal::VCUServoID::LeftFrontServo || servo.servoid() == hal::VCUServoID::RightFrontServo) {
                    EXPECT_TRUE(servo.servoangleradians() > 0);
                    existServoData = true;
                } else if (servo.servoid() == hal::VCUServoID::LeftRearServo || servo.servoid() == hal::VCUServoID::RightRearServo) {
                    EXPECT_TRUE(servo.servoangleradians() < 0);
                    existServoData = true;
                }
            }
        }
        t2 = std::chrono::high_resolution_clock::now();
        fp_ms = t2 - t1;
    }
    EXPECT_TRUE(existServoData);

    // wait until the rest of the trajectory command finished being executed.
    std::this_thread::sleep_for(std::chrono::seconds(5));
}

///
/// @brief Checks the correctness of the signs of the servo telemetry
/// @details It checks the signs of the servo telemetry while the vehicle is moving.
///          When the curvature center is on the right outside the vehicle,
///          the front wheels should have positive angle, whereas the rear wheels have negative angles.
///          Opposite, when the curvature center is on the left side.
///
TEST(TelemetryDataValidation, checkWheelServoSignMovingLeft) {
    LOG(INFO) << "LEFT";

    unity_plugins::ConnectionsReader reader(DEFAULT_CONNECTIONS_FILE);
    const auto telemetryAddress = reader.fullAddress(mercury::topic::telemetry);
    const auto trajectorySubscriberAddress = reader.fullAddress(mercury::topic::trajectory);

    zmq::context_t context(1);
    sendTrajectoryCommandMovingLeft(context, trajectorySubscriberAddress);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    net::ZMQProtobufSubscriber<hal::VCUTelemetryEnvelope> vcuTelemSub(context, telemetryAddress, "telemetry", 1);

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> fp_ms = t2 - t1;

    bool existServoData = false;
    while (fp_ms < std::chrono::seconds(5)) {

        if (vcuTelemSub.poll(std::chrono::milliseconds(10))) {
            hal::VCUTelemetryEnvelope vcuTelemEnvelope;
            vcuTelemSub.recv(vcuTelemEnvelope);

            if (vcuTelemEnvelope.has_servo()) {
                auto servo = vcuTelemEnvelope.servo();
                if (servo.servoid() == hal::VCUServoID::LeftFrontServo || servo.servoid() == hal::VCUServoID::RightFrontServo) {
                    EXPECT_TRUE(servo.servoangleradians() < 0);
                    existServoData = true;
                } else if (servo.servoid() == hal::VCUServoID::LeftRearServo || servo.servoid() == hal::VCUServoID::RightRearServo) {
                    EXPECT_TRUE(servo.servoangleradians() > 0);
                    existServoData = true;
                }
            }
        }
        t2 = std::chrono::high_resolution_clock::now();
        fp_ms = t2 - t1;
    }
    EXPECT_TRUE(existServoData);

    // wait until the rest of the trajectory command finished being executed.
    std::this_thread::sleep_for(std::chrono::seconds(5));
}
