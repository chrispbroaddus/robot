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
#include "packages/unity_plugins/proto/unity_telemetry_envelope.pb.h"

namespace {
constexpr auto DEFAULT_CONNECTIONS_FILE = "packages/unity_simulator/test/connections.default.json";
}

void sendTrajectoryCommandMovingStraightForward(zmq::context_t& context, const std::string& trajectorySubscriberAddress) {

    net::ZMQProtobufReqClient<hal::VCUCommandEnvelope, hal::VCUCommandResponse> client(context, trajectorySubscriberAddress, 1000, 1000);

    int counter = 1;

    hal::VCUTrajectoryCommand* trajectory = new hal::VCUTrajectoryCommand();

    constexpr uint64_t gpsTimestampOffsetInNanos = 1e6;
    auto gpsTimestamp = core::chrono::gps::wallClockInNanoseconds();

    uint32_t millisecondPerElement = 500;
    int numElements = 20;
    for (int i = 0; i < numElements; i++) {
        auto element = trajectory->add_segments();

        uint64_t futureTimestamp
            = gpsTimestamp.count() + (uint64_t)i * millisecondPerElement * (uint64_t)1000000 + gpsTimestampOffsetInNanos;
        LOG(INFO) << "futureTimestamp : " << std::to_string(futureTimestamp);
        element->mutable_targetstarttime()->set_nanos(futureTimestamp);

        hal::VCUArcDriveSegment arcDriveSegment;
        if (i != numElements - 1) {
            arcDriveSegment.set_linearvelocitymeterspersecond(1);
            arcDriveSegment.set_curvatureinversemeters(0);
        } else {
            arcDriveSegment.set_linearvelocitymeterspersecond(0);
            arcDriveSegment.set_curvatureinversemeters(0);
        }
        *element->mutable_arcdrive() = arcDriveSegment;
    }

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

///
/// @brief The unity-functional-test compares the distance based on the wheel odometry vs.
///        the ground truth data reported by unity-telemetry.
///
TEST(TelemetryDataValidation, compareGroundTruthPosition) {

    // limit on the expected error due to the missing data, slippery surface, etc.
    constexpr float maxAllowedDiff = 0.3;

    // The vehicle is expected to received the trajectory command and move meaningful distance.
    constexpr float minRequiredTravelDistance = 3;

    unity_plugins::ConnectionsReader reader(DEFAULT_CONNECTIONS_FILE);
    const auto telemetryAddress = reader.fullAddress(mercury::topic::telemetry);
    const auto trajectorySubscriberAddress = reader.fullAddress(mercury::topic::trajectory);
    const auto groundTruthTelemetryAddress = reader.fullAddress(mercury::topic::ground_truth_telemetry);

    zmq::context_t context(1);
    sendTrajectoryCommandMovingStraightForward(context, trajectorySubscriberAddress);
    net::ZMQProtobufSubscriber<hal::VCUTelemetryEnvelope> vcuTelemSub(context, telemetryAddress, "telemetry", 1);
    net::ZMQProtobufSubscriber<unity_plugins::UnityTelemetryEnvelope> unityTelemSub(context, groundTruthTelemetryAddress, "telemetry", 1);

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> fp_ms = t2 - t1;

    bool isFirstArrivedVehiclePose = true;

    std::unordered_map<int, float> odometryDistance;
    odometryDistance[(int)hal::VCUWheelID::LeftFrontWheel] = 0;
    odometryDistance[(int)hal::VCUWheelID::LeftMiddleWheel] = 0;
    odometryDistance[(int)hal::VCUWheelID::LeftRearWheel] = 0;
    odometryDistance[(int)hal::VCUWheelID::RightFrontWheel] = 0;
    odometryDistance[(int)hal::VCUWheelID::RightMiddleWheel] = 0;
    odometryDistance[(int)hal::VCUWheelID::RightRearWheel] = 0;

    float initX, initY, initZ;

    while (fp_ms < std::chrono::seconds(10)) {

        if (isFirstArrivedVehiclePose && unityTelemSub.poll(std::chrono::milliseconds(1))) {
            unity_plugins::UnityTelemetryEnvelope unityTelemEnvelope;
            unityTelemSub.recv(unityTelemEnvelope);

            if (unityTelemEnvelope.has_vehiclepose()) {
                auto vehiclepose = unityTelemEnvelope.vehiclepose();
                initX = vehiclepose.transformations().translationx();
                initY = vehiclepose.transformations().translationy();
                initZ = vehiclepose.transformations().translationz();
                isFirstArrivedVehiclePose = false;
            }
        }

        // Drop out unnecessary unity telemetry data
        if (unityTelemSub.poll(std::chrono::microseconds(1))) {
            unity_plugins::UnityTelemetryEnvelope unityTelemEnvelope;
            unityTelemSub.recv(unityTelemEnvelope);
        }

        if (vcuTelemSub.poll(std::chrono::milliseconds(10))) {
            hal::VCUTelemetryEnvelope vcuTelemEnvelope;
            vcuTelemSub.recv(vcuTelemEnvelope);
            if (vcuTelemEnvelope.has_wheelencoder()) {
                auto wheelTelemetry = vcuTelemEnvelope.wheelencoder();
                odometryDistance[(int)wheelTelemetry.wheelid()] += wheelTelemetry.lineardisplacementmeters();
            }
        }
        t2 = std::chrono::high_resolution_clock::now();
        fp_ms = t2 - t1;
    }

    bool receivedFinalVehiclePoseTelemetry = false;
    t1 = std::chrono::high_resolution_clock::now();
    t2 = std::chrono::high_resolution_clock::now();
    fp_ms = t2 - t1;
    while (fp_ms < std::chrono::milliseconds(100)) {
        if (unityTelemSub.poll(std::chrono::milliseconds(1))) {
            unity_plugins::UnityTelemetryEnvelope unityTelemEnvelope;
            unityTelemSub.recv(unityTelemEnvelope);

            if (unityTelemEnvelope.has_vehiclepose()) {
                auto vehiclepose = unityTelemEnvelope.vehiclepose();
                float groundTruthDistance = std::sqrt(std::pow(initX - vehiclepose.transformations().translationx(), 2)
                    + std::pow(initY - vehiclepose.transformations().translationy(), 2)
                    + std::pow(initZ - vehiclepose.transformations().translationz(), 2));

                // Check if the vehicle is moved enough.
                EXPECT_TRUE(groundTruthDistance > minRequiredTravelDistance);

                // Check the final distance is consistent with the wheel-odometry based distance
                EXPECT_NEAR(odometryDistance[(int)hal::VCUWheelID::LeftFrontWheel], groundTruthDistance, maxAllowedDiff);
                EXPECT_NEAR(odometryDistance[(int)hal::VCUWheelID::LeftMiddleWheel], groundTruthDistance, maxAllowedDiff);
                EXPECT_NEAR(odometryDistance[(int)hal::VCUWheelID::LeftRearWheel], groundTruthDistance, maxAllowedDiff);
                EXPECT_NEAR(odometryDistance[(int)hal::VCUWheelID::RightFrontWheel], groundTruthDistance, maxAllowedDiff);
                EXPECT_NEAR(odometryDistance[(int)hal::VCUWheelID::RightMiddleWheel], groundTruthDistance, maxAllowedDiff);
                EXPECT_NEAR(odometryDistance[(int)hal::VCUWheelID::RightRearWheel], groundTruthDistance, maxAllowedDiff);
                receivedFinalVehiclePoseTelemetry = true;
                break;
            }
        }
        t2 = std::chrono::high_resolution_clock::now();
        fp_ms = t2 - t1;
    }
    EXPECT_TRUE(receivedFinalVehiclePoseTelemetry);
}
