#include "glog/logging.h"
#include "packages/core/include/chrono.h"
#include "packages/hal/include/drivers/joysticks/sdl2/joystick_device.h"
#include "packages/hal/include/drivers/vcu/vcu_device_interface.h"
#include "packages/net/include/zmq_req_client.h"
#include "packages/net/include/zmq_topic_sub.h"

#include <atomic>

using namespace hal;
using namespace hal::vcu;
using namespace net;

std::atomic_bool run;
void sig_handler(int s) {
    if (s == 2)
        run = false;
}

int main() {

    zmq::context_t context = zmq::context_t(1);
    LOG(INFO) << "Creating Subscriber\n";
    ZMQProtobufSubscriber<VCUTelemetryEnvelope> telemetrySub(context, "tcp://localhost:5801", "telemetry", 1);
    LOG(INFO) << "Creating client\n";
    ZMQProtobufReqClient<VCUCommandEnvelope, VCUCommandResponse> client(context, "tcp://localhost:5802", 1000, 100);

    JoystickDevice joystick(0);

    signal(SIGINT, sig_handler);

    LOG(INFO) << "Starting transmission\n";
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
            startTime1->set_nanos(timestamp.count() + (uint64_t)10000000);
            trajectorySegment1->set_allocated_targetstarttime(startTime1);
            *trajectorySegment1->mutable_arcdrive() = arcDriveSegment;

            trajectorySegment2 = trajectoryCommand->add_segments();
            startTime2->set_nanos(timestamp.count() + (uint64_t)150000000);
            trajectorySegment2->set_allocated_targetstarttime(startTime2);
            *trajectorySegment2->mutable_arcdrive() = arcDriveSegment;

            trajectorySegment3 = trajectoryCommand->add_segments();
            startTime3->set_nanos(timestamp.count() + (uint64_t)200000000);
            trajectorySegment3->set_allocated_targetstarttime(startTime3);
            *trajectorySegment3->mutable_arcdrive() = arcDriveSegment;

            commandEnvelope.set_sequencenumber(sequenceNum);
            commandEnvelope.set_allocated_trajectorycommand(trajectoryCommand);

            LOG(INFO) << "Sending trajectory. velocity: " << velocity << " curvature: " << curvature;
            if (client.send(commandEnvelope)) {
                if (client.recv(commandResponse)) {
                    LOG(INFO) << "Trajectory Response: " << commandResponse.DebugString();
                }
            }
            sequenceNum++;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
