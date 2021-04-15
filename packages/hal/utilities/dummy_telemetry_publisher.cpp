#include "packages/hal/proto/vcu_telemetry_envelope.pb.h"

#include "glog/logging.h"

#include <atomic>
#include <chrono>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>

std::atomic_bool run;
void sig_handler(int s) {
    if (s == 2)
        run = false;
}

class DummyTelemetryPublisher {
public:
    DummyTelemetryPublisher() {
        m_telemetrySocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (m_telemetrySocket < 0) {
            LOG(ERROR) << "Cannot create telemetry socket";
            throw std::runtime_error("Cannot create telemetry socket");
        }

        memset((char*)&m_telemetryAddress, 0, sizeof(m_telemetryAddress));
        m_telemetryAddress.sin_family = AF_INET;
        m_telemetryAddress.sin_port = htons(10002);
        m_telemetryAddress.sin_addr.s_addr = htonl(INADDR_ANY);

        if (connect(m_telemetrySocket, (struct sockaddr*)&m_telemetryAddress, sizeof(m_telemetryAddress)) < 0) {
            LOG(ERROR) << "Failed to connect to host on telemetry channel";
            throw std::runtime_error("Failed to connect to host on telemetry channel");
        }
        LOG(INFO) << "Created telemetry socket";
    }

    ~DummyTelemetryPublisher() { close(m_telemetrySocket); }

    bool sendTelemetry() {

        hal::VCUTelemetryEnvelope telemetryEnvelope;

        for (int ctr = 0; ctr < 20; ctr++) {
            telemetryEnvelope.mutable_sendtimestamp()->set_nanos(m_counter++);
            telemetryEnvelope.mutable_wheelencoder()->mutable_periodstarthardwaretimestamp()->set_nanos(100);
            telemetryEnvelope.mutable_wheelencoder()->mutable_periodendhardwaretimestamp()->set_nanos(100);
            telemetryEnvelope.mutable_wheelencoder()->mutable_periodendsystemtimestamp()->set_nanos(100);
            telemetryEnvelope.mutable_wheelencoder()->set_lineardisplacementmeters(1);
            telemetryEnvelope.mutable_wheelencoder()->set_halleffectnoisemeasurementgapmicroseconds(1);
            telemetryEnvelope.mutable_wheelencoder()->set_wheelid(hal::VCUWheelID::LeftFrontWheel);

            /// Send serialized telemetry protobuf
            std::string telemetry = telemetryEnvelope.SerializeAsString();
            if (sendto(m_telemetrySocket, telemetry.c_str(), telemetry.size(), 0, (struct sockaddr*)&m_telemetryAddress,
                    sizeof(m_telemetryAddress))
                == -1) {
                LOG_EVERY_N(ERROR, 1000) << "Send telemetry failed";
                return false;
            }
        }
        return true;
    }

private:
    int m_bufferSize = 2048;
    int m_telemetrySocket;
    struct sockaddr_in m_telemetryAddress;
    uint64_t m_counter = 0;
};

int main(int /*argc*/, char** /*argv*/) {

    DummyTelemetryPublisher telemetryPublisher;

    signal(SIGINT, sig_handler);

    run = true;
    while (run) {
        telemetryPublisher.sendTelemetry();
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}
