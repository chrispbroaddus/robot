#include "packages/core/include/chrono.h"
#include "packages/hal/include/drivers/joysticks/sdl2/joystick_device.h"
#include "packages/hal/include/drivers/vcu/udp_socket/udp_socket_vcu_driver.h"

#include "glog/logging.h"

#include <atomic>

using namespace hal;
using namespace hal::vcu;

std::atomic_bool run;
void sig_handler(int s) {
    if (s == 2)
        run = false;
}

int main(int argc, char** argv) {

    if (argc < 4) {
        LOG(ERROR) << "Usage: udp_echo_test <vcuAddress> <commandPort> <telemetryPort>";
        return 0;
    }

    const int commandPort = std::stoi(argv[2]);
    const int telemetryPort = std::stoi(argv[3]);
    LOG(INFO) << "Command Port: " << commandPort << " Telemetry Port: " << telemetryPort;
    UdpSocketVcuDriver udpVCUDriver(argv[1], commandPort, telemetryPort);

    JoystickDevice joystick(0);

    run = true;
    uint64_t sequenceNum = 1;
    while (run) {
        JoystickSample joystickSample;

        if (joystick.poll()) {
            joystick.capture(joystickSample);

            float axis4Val = joystickSample.axis(4);
            float axis0val = joystickSample.axis(0);
            float velocity;
            if (axis4Val < 0.05f && axis4Val > -0.05f)
                velocity = 0;
            else
                velocity = axis4Val * -1.45f; /// Max: 1.5 m/s
            float curvature; /// Max: 10 m^-1
            /*if (axis0val > 0.95)
                curvature = 10;
            else if (axis0val < -0.95)
                curvature = -10;
            else
                curvature = axis0val;*/
            curvature = 2 * axis0val;

            VCUCommandEnvelope commandEnvelope;
            VCUCommandResponse commandResponse;
            VCUTrajectoryCommand* trajectoryCommand = new VCUTrajectoryCommand();
            VCUTrajectorySegment* trajectorySegment1;
            core::SystemTimestamp* startTime1 = new core::SystemTimestamp();
            VCUTrajectorySegment* trajectorySegment2;
            core::SystemTimestamp* startTime2 = new core::SystemTimestamp();
            VCUTrajectorySegment* trajectorySegment3;
            core::SystemTimestamp* startTime3 = new core::SystemTimestamp();

            const std::chrono::nanoseconds timestamp = core::chrono::gps::wallClockInNanoseconds();

            VCUArcDriveSegment arcDriveSegment;
            arcDriveSegment.set_linearvelocitymeterspersecond(velocity);
            arcDriveSegment.set_curvatureinversemeters(curvature);

            trajectorySegment1 = trajectoryCommand->add_segments();
            startTime1->set_nanos(timestamp.count() + (uint64_t)50000000);
            trajectorySegment1->set_allocated_targetstarttime(startTime1);
            *trajectorySegment1->mutable_arcdrive() = arcDriveSegment;

            trajectorySegment2 = trajectoryCommand->add_segments();
            startTime2->set_nanos(timestamp.count() + (uint64_t)100000000);
            trajectorySegment2->set_allocated_targetstarttime(startTime2);
            *trajectorySegment2->mutable_arcdrive() = arcDriveSegment;

            trajectorySegment3 = trajectoryCommand->add_segments();
            startTime3->set_nanos(timestamp.count() + (uint64_t)200000000);
            trajectorySegment3->set_allocated_targetstarttime(startTime3);
            *trajectorySegment3->mutable_arcdrive() = arcDriveSegment;

            commandEnvelope.set_sequencenumber(sequenceNum);
            commandEnvelope.set_allocated_trajectorycommand(trajectoryCommand);

            LOG(INFO) << commandEnvelope.DebugString();
            if (udpVCUDriver.send(commandEnvelope, commandResponse)) {
                LOG(INFO) << commandResponse.DebugString();
            }
            sequenceNum++;
        }

        if (udpVCUDriver.poll(10)) {
            VCUTelemetryEnvelope telemetryEnvelope;
            udpVCUDriver.capture(telemetryEnvelope);
            LOG(INFO) << telemetryEnvelope.DebugString();
        }
    }
}
