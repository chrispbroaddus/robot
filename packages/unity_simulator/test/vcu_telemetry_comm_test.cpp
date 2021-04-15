#include "glog/logging.h"
#include "gtest/gtest.h"

#include <atomic>
#include <cmath>
#include <thread>

#include "packages/net/include/zmq_topic_sub.h"
#include "packages/unity_plugins/connections_reader/include/connections_reader.h"
#include "packages/unity_plugins/vcu_telemetry_publisher/include/vcu_telemetry_publisher.h"

namespace {
constexpr auto DEFAULT_CONNECTIONS_FILE = "packages/unity_simulator/test/connections.default.json";
}

///
/// @brief Check if it can receive any telemetry data from the simulator.
///
TEST(SimulatorVcuTelemetryCommunication, telemetryExistence) {

    unity_plugins::ConnectionsReader reader(DEFAULT_CONNECTIONS_FILE);
    const auto telemetryAddress = reader.fullAddress(mercury::topic::telemetry);

    zmq::context_t context(1);
    net::ZMQProtobufSubscriber<hal::VCUTelemetryEnvelope> subscriber(context, telemetryAddress, "telemetry", 1);

    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> fp_ms = t2 - t1;

    bool dataReceived = false;
    while (fp_ms < std::chrono::seconds(60)) {
        if (subscriber.poll(std::chrono::milliseconds(10))) {
            hal::VCUTelemetryEnvelope envelope;
            if (subscriber.recv(envelope)) {
                dataReceived = true;
                break;
            }
        }
        t2 = std::chrono::high_resolution_clock::now();
        fp_ms = t2 - t1;
    }
    EXPECT_TRUE(dataReceived);
}
