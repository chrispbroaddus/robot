#include "glog/logging.h"
#include "gtest/gtest.h"

#include "packages/core/test/common.h"
#include "packages/hal/proto/vcu_command_response.pb.h"
#include "packages/net/include/zmq_req_client.h"
#include "packages/unity_plugins/simulated_vcu/include/simulated_vcu.h"

using namespace unity_plugins;

using core::test::UniquePortProvider;
UniquePortProvider provider;

struct CreateVcuTrajectoryOptions {
    int numSegments;
    uint64_t elementPeriodInMilliSec;
    float linearVelocity;
    float curvature;
};

hal::VCUCommandEnvelope createValidVcuCommand(const CreateVcuTrajectoryOptions& options) {
    hal::VCUTrajectoryCommand* trajectory = new hal::VCUTrajectoryCommand();

    constexpr uint64_t gpsTimestampOffsetInNanos = 1e7;
    auto gpsTimestamp = core::chrono::gps::wallClockInNanoseconds();

    VLOG(2) << __PRETTY_FUNCTION__ << " ... gpsTimestamp: " << gpsTimestamp.count() << std::endl;

    constexpr uint64_t kMilliToNano = 1e6;
    for (int i = 0; i < options.numSegments; i++) {
        auto element = trajectory->add_segments();

        uint64_t futureTimestamp
            = gpsTimestamp.count() + (uint64_t)i * options.elementPeriodInMilliSec * kMilliToNano + gpsTimestampOffsetInNanos;
        element->mutable_targetstarttime()->set_nanos(futureTimestamp);

        hal::VCUArcDriveSegment arcDriveSegment;
        if (i != 9) {
            arcDriveSegment.set_linearvelocitymeterspersecond(options.linearVelocity);
            arcDriveSegment.set_curvatureinversemeters(options.curvature);
        } else {
            arcDriveSegment.set_linearvelocitymeterspersecond(0);
            arcDriveSegment.set_curvatureinversemeters(options.curvature);
        }
        *element->mutable_arcdrive() = arcDriveSegment;
    }
    hal::VCUCommandEnvelope envelope;
    envelope.set_sequencenumber(1);
    envelope.set_allocated_trajectorycommand(trajectory);

    return envelope;
}

hal::VCUCommandEnvelope createInvalidTimestampedVcuCommand(const CreateVcuTrajectoryOptions& options) {

    hal::VCUTrajectoryCommand* trajectory = new hal::VCUTrajectoryCommand();

    constexpr uint64_t gpsTimestampOffsetInNanos = 1e7;
    auto gpsTimestamp = core::chrono::gps::wallClockInNanoseconds();

    std::cout << "gpsTimestamp: " << gpsTimestamp.count() << std::endl;

    constexpr uint64_t kMilliToNano = 1e6;
    for (int i = 0; i < options.numSegments; i++) {
        auto element = trajectory->add_segments();

        // The line below is intentionally creating a wrongly-reverse-timestamped trajectory for unit tests.
        uint64_t futureTimestamp
            = gpsTimestamp.count() - (uint64_t)i * options.elementPeriodInMilliSec * kMilliToNano + gpsTimestampOffsetInNanos;
        element->mutable_targetstarttime()->set_nanos(futureTimestamp);

        hal::VCUArcDriveSegment arcDriveSegment;
        if (i != 9) {
            arcDriveSegment.set_linearvelocitymeterspersecond(options.linearVelocity);
            arcDriveSegment.set_curvatureinversemeters(options.curvature);
        } else {
            arcDriveSegment.set_linearvelocitymeterspersecond(0);
            arcDriveSegment.set_curvatureinversemeters(options.curvature);
        }

        *element->mutable_arcdrive() = arcDriveSegment;
    }
    hal::VCUCommandEnvelope envelope;
    envelope.set_sequencenumber(1);
    envelope.set_allocated_trajectorycommand(trajectory);

    return envelope;
}

void sendTrajectorySignal(bool& sendSuccess, hal::VCUCommandResponse& commandResponse, const std::string& address,
    std::function<hal::VCUCommandEnvelope(const CreateVcuTrajectoryOptions&)> vcuCommandGenerateFunc,
    const CreateVcuTrajectoryOptions& options) {

    zmq::context_t context(1);
    net::ZMQProtobufReqClient<hal::VCUCommandEnvelope, hal::VCUCommandResponse> client(context, address, 1000, 1000);

    fprintf(stdout, "Trying binding a socket.\n");

    hal::VCUCommandEnvelope command = vcuCommandGenerateFunc(options);
    sendSuccess = client.send(command);

    client.recv(commandResponse);
}

void sendTrajecorySignal(
    bool& sendSuccess, hal::VCUCommandResponse& commandResponse, const std::string& address, const CreateVcuTrajectoryOptions& options) {
    std::function<hal::VCUCommandEnvelope(const CreateVcuTrajectoryOptions&)> f = createValidVcuCommand;
    sendTrajectorySignal(sendSuccess, commandResponse, address, f, options);
}

void sendInvalidTimestampedTrajecorySignal(
    bool& sendSuccess, hal::VCUCommandResponse& commandResponse, std::string& address, const CreateVcuTrajectoryOptions& options) {
    std::function<hal::VCUCommandEnvelope(const CreateVcuTrajectoryOptions&)> f = createInvalidTimestampedVcuCommand;
    sendTrajectorySignal(sendSuccess, commandResponse, address, f, options);
}

void setDefaultVehicleCalibration(VehicleCalibration& vehicleCalibration) {
    vehicleCalibration.center.x = 0;
    vehicleCalibration.center.y = 0;
    vehicleCalibration.frontLeft.x = -1;
    vehicleCalibration.frontLeft.y = 1;
    vehicleCalibration.frontRight.x = 1;
    vehicleCalibration.frontRight.y = 1;
    vehicleCalibration.middleLeft.x = -1;
    vehicleCalibration.middleLeft.y = 0;
    vehicleCalibration.middleRight.x = 1;
    vehicleCalibration.middleRight.y = 0;
    vehicleCalibration.rearLeft.x = -1;
    vehicleCalibration.rearLeft.y = -1;
    vehicleCalibration.rearRight.x = 1;
    vehicleCalibration.rearRight.y = -1;
}

TEST(SimulatedVCU, ExpectZeroTorquesInConvergence) {
    const int portNumber = provider.next_port();
    constexpr float PIDKu = 500;
    constexpr float PIDTu = 1;

    VehicleCalibration vehicleCalibration;
    setDefaultVehicleCalibration(vehicleCalibration);

    std::string address = "tcp://127.0.0.1:" + std::to_string(portNumber);
    SimulatedVCU simulatedVCU(address, PIDKu, PIDTu, vehicleCalibration);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    CreateVcuTrajectoryOptions options;
    options.numSegments = 10;
    options.elementPeriodInMilliSec = 5000;
    options.linearVelocity = 1.0f;
    options.curvature = 1.f / 10.f;

    bool success;
    hal::VCUCommandResponse response;
    sendTrajecorySignal(success, response, address, options);
    EXPECT_TRUE(success);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // When the current vehicle velocity is at desired velocity, no force should be applied.
    SimulatedVCU::WheelTelemetry wheelTelemetry;
    wheelTelemetry.centerBodyLinearVelocity = options.linearVelocity;
    wheelTelemetry.gpsSystemtimeCount = core::chrono::gps::wallClockInNanoseconds().count();

    simulatedVCU.update(wheelTelemetry);

    const auto& controlCommand1 = simulatedVCU.getVehicleControlCommand();
    EXPECT_EQ(controlCommand1.frontLeftWheelTorque, 0);
    EXPECT_EQ(controlCommand1.frontRightWheelTorque, 0);
    EXPECT_EQ(controlCommand1.middleLeftWheelTorque, 0);
    EXPECT_EQ(controlCommand1.middleRightWheelTorque, 0);
    EXPECT_EQ(controlCommand1.rearLeftWheelTorque, 0);
    EXPECT_EQ(controlCommand1.rearRightWheelTorque, 0);
}

TEST(SimulatedVCU, ExpectNonZeroTorquesInBeginning) {
    const int portNumber = provider.next_port();
    constexpr float PIDKu = 500;
    constexpr float PIDTu = 1;

    VehicleCalibration vehicleCalibration;
    setDefaultVehicleCalibration(vehicleCalibration);

    std::string address = "tcp://127.0.0.1:" + std::to_string(portNumber);
    SimulatedVCU simulatedVCU(address, PIDKu, PIDTu, vehicleCalibration);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    CreateVcuTrajectoryOptions options;
    options.numSegments = 10;
    options.elementPeriodInMilliSec = 5000;
    options.linearVelocity = 1.0f;
    options.curvature = 1.f / 10.f;

    bool success;
    hal::VCUCommandResponse response;
    sendTrajecorySignal(success, response, address, options);
    EXPECT_TRUE(success);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // When the current vehicle velocity is at desired velocity, no force should be applied.
    SimulatedVCU::WheelTelemetry wheelTelemetry;
    wheelTelemetry.centerBodyLinearVelocity = 0;
    wheelTelemetry.gpsSystemtimeCount = core::chrono::gps::wallClockInNanoseconds().count();

    simulatedVCU.update(wheelTelemetry);

    const auto& controlCommand1 = simulatedVCU.getVehicleControlCommand();
    EXPECT_NE(controlCommand1.frontLeftWheelTorque, 0);
    EXPECT_NE(controlCommand1.frontRightWheelTorque, 0);
    EXPECT_NE(controlCommand1.middleLeftWheelTorque, 0);
    EXPECT_NE(controlCommand1.middleRightWheelTorque, 0);
    EXPECT_NE(controlCommand1.rearLeftWheelTorque, 0);
    EXPECT_NE(controlCommand1.rearRightWheelTorque, 0);
}

///
/// @brief The command arrived later able to overwrite the old command
///        for the redundant timespans.
///
TEST(SimulatedVCU, InterruptibleCommand) {
    const int portNumber = provider.next_port();
    constexpr float PIDKu = 500;
    constexpr float PIDTu = 1;

    VehicleCalibration vehicleCalibration;
    setDefaultVehicleCalibration(vehicleCalibration);

    std::string address = "tcp://127.0.0.1:" + std::to_string(portNumber);
    SimulatedVCU simulatedVCU(address, PIDKu, PIDTu, vehicleCalibration);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    CreateVcuTrajectoryOptions options1;
    options1.numSegments = 10;
    options1.elementPeriodInMilliSec = 5000;
    options1.linearVelocity = 1.0f;
    options1.curvature = 1.f / 10.f;

    bool success;
    hal::VCUCommandResponse response;
    sendTrajecorySignal(success, response, address, options1);
    EXPECT_TRUE(success);

    CreateVcuTrajectoryOptions options2;
    options2.numSegments = 10;
    options2.elementPeriodInMilliSec = 5000;
    options2.linearVelocity = 1.0f;
    options2.curvature = 1.f / 10.f;

    sendTrajecorySignal(success, response, address, options2);
    EXPECT_TRUE(success);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // When the current vehicle velocity is at desired velocity, no force should be applied.
    SimulatedVCU::WheelTelemetry wheelTelemetry;
    wheelTelemetry.centerBodyLinearVelocity = options2.linearVelocity;
    wheelTelemetry.gpsSystemtimeCount = core::chrono::gps::wallClockInNanoseconds().count();

    simulatedVCU.update(wheelTelemetry);

    const auto& controlCommand1 = simulatedVCU.getVehicleControlCommand();
    EXPECT_EQ(controlCommand1.frontLeftWheelTorque, 0);
    EXPECT_EQ(controlCommand1.frontRightWheelTorque, 0);
    EXPECT_EQ(controlCommand1.middleLeftWheelTorque, 0);
    EXPECT_EQ(controlCommand1.middleRightWheelTorque, 0);
    EXPECT_EQ(controlCommand1.rearLeftWheelTorque, 0);
    EXPECT_EQ(controlCommand1.rearRightWheelTorque, 0);
}

///
/// Test if the command is rejected if wrong trajectory command is sent
///
TEST(SimulatedVCU, ExpectCommandRejectedWithInvalidTimestampedTrajectory) {
    const int portNumber = provider.next_port();
    constexpr float PIDKu = 500;
    constexpr float PIDTu = 1;

    VehicleCalibration vehicleCalibration;
    setDefaultVehicleCalibration(vehicleCalibration);

    std::string address = "tcp://127.0.0.1:" + std::to_string(portNumber);
    SimulatedVCU simulatedVCU(address, PIDKu, PIDTu, vehicleCalibration);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    CreateVcuTrajectoryOptions options;
    options.numSegments = 10;
    options.elementPeriodInMilliSec = 5000;
    options.linearVelocity = 1.0f;
    options.curvature = 1.f / 10.f;

    bool sendSuccess;
    hal::VCUCommandResponse response;
    sendInvalidTimestampedTrajecorySignal(sendSuccess, response, address, options);
    EXPECT_TRUE(sendSuccess);
    EXPECT_EQ(response.disposition(), hal::VCUCommandDisposition::CommandRejected);
}
