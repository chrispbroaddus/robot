#include "packages/hal/proto/vcu_telemetry_envelope.pb.h"
#include "packages/net/include/zmq_topic_sub.h"

#include "glog/logging.h"

#include <atomic>
#include <thread>

std::atomic_bool run;
void sig_handler(int s) {
    if (s == 2)
        run = false;
}

int main(int /*argc*/, char** /*argv*/) {

    zmq::context_t context(1);
    net::ZMQProtobufSubscriber<hal::VCUTelemetryEnvelope> telemetrySubscriber(context, "tcp://localhost:5801", "telemetry", 10);
    uint64_t prevId = -1;
    uint64_t capturedFrames = 0;
    uint64_t droppedFrames = 0;

    run = true;
    while (run) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        while (telemetrySubscriber.poll(std::chrono::milliseconds(10))) {
            hal::VCUTelemetryEnvelope telemetryEnvelope;
            telemetrySubscriber.recv(telemetryEnvelope);
            capturedFrames++;
            if (prevId != telemetryEnvelope.sendtimestamp().nanos() - 1) {
                LOG(INFO) << "Dropped Frame. Captured: " << capturedFrames << " Dropped: " << droppedFrames;
                LOG(INFO) << "prevId: " << prevId << " newId: " << telemetryEnvelope.sendtimestamp().nanos();
                droppedFrames += telemetryEnvelope.sendtimestamp().nanos() - prevId - 1;
                LOG(INFO) << "Dropped Frame. Captured: " << capturedFrames << " Dropped: " << droppedFrames;
            }
            prevId = telemetryEnvelope.sendtimestamp().nanos();
        }
        LOG_EVERY_N(INFO, 1000) << "Captured: " << capturedFrames << " Dropped: " << droppedFrames;
    }
    LOG(INFO) << "Total frames dropped: " << droppedFrames;
}
