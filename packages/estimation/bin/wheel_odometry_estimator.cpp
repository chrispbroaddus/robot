#include <math.h>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <unistd.h>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "sophus/se3.hpp"

#include "packages/core/proto/geometry.pb.h"
#include "packages/estimation/estimator.h"
#include "packages/net/include/zmq_topic_sub.h"

DEFINE_string(wheel_odometry_options, "", "Odometry options");

int main(int argc, char** argv) {
    gflags::SetUsageMessage("Wheel odometry estimator");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    using estimation::WheelOdometryEstimator;
    using estimation::WheelOdometryOptions;

    WheelOdometryOptions options;
    if (!FLAGS_wheel_odometry_options.empty()) {
        CHECK(planning::loadOptions(FLAGS_wheel_odometry_options, options));
    } else {
        using estimation::loadDefaultWheelOdometryOptions;
        options = loadDefaultWheelOdometryOptions();
    }
    LOG(INFO) << "Using options:";
    LOG(INFO) << options.DebugString();
    std::string address = options.base_options().subscribeaddress() + ":" + std::to_string(options.base_options().subscribeport());
    zmq::context_t context(1);
    net::ZMQProtobufSubscriber<hal::VCUTelemetryEnvelope> subscriber(context, address, "telemetry", 1);
    std::unique_ptr<WheelOdometryEstimator> odometer(new WheelOdometryEstimator(options));

    while (true) {
        if (subscriber.poll()) {
            hal::VCUTelemetryEnvelope envelope;
            if (subscriber.recv(envelope)) {
                odometer->update(envelope);
            }
        }
    }
    return EXIT_SUCCESS;
}
