#include "packages/hal/include/drivers/vcu/udp_socket/udp_socket_vcu_driver.h"

#include "glog/logging.h"

#include <atomic>

std::atomic_bool run;
void sig_handler(int s) {
    if (s == 2)
        run = false;
}

int main(int /*argc*/, char** /*argv*/) {

    hal::vcu::UdpSocketVcuDriver vcuDriver("0.0.0.0", 10001, 10002);
    uint64_t prevId = -1;
    uint64_t capturedFrames = 0;
    uint64_t droppedFrames = 0;

    run = true;
    while (run) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        while (vcuDriver.poll(1000)) {
            hal::VCUTelemetryEnvelope telemetryEnvelope;
            vcuDriver.capture(telemetryEnvelope);
            capturedFrames++;
            if (prevId != telemetryEnvelope.sendtimestamp().nanos() - 1) {
                LOG(INFO) << "Dropped Frame";
                droppedFrames++;
            }
            prevId = telemetryEnvelope.sendtimestamp().nanos();
        }
        LOG_EVERY_N(INFO, 1000) << "Captured: " << capturedFrames << " Dropped: " << droppedFrames;
    }
    LOG(INFO) << "Total frames dropped: " << droppedFrames;
}
