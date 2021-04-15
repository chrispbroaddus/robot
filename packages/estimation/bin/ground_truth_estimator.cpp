#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gflags/gflags.h"
#include "sophus/se3.hpp"

#include "packages/core/proto/geometry.pb.h"
#include "packages/estimation/estimator.h"
#include "packages/net/include/zmq_topic_sub.h"

int main(int argc, char** argv) {
    gflags::SetUsageMessage("Ground-truth estimator");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    using estimation::GroundTruthEstimator;
    using estimation::GroundTruthEstimatorOptions;

    GroundTruthEstimatorOptions options;
    options.mutable_base_options()->set_subscribeaddress("tcp://localhost");
    options.mutable_base_options()->set_subscribeport(7501);
    options.mutable_base_options()->set_publishaddress("tcp://127.0.0.1");
    options.mutable_base_options()->set_publishport(7101);

    std::unique_ptr<GroundTruthEstimator> estimator;
    estimator.reset(new GroundTruthEstimator(options));

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return EXIT_SUCCESS;
}
