#include <fstream>

#include "glog/logging.h"
#include "google/protobuf/text_format.h"

#include "packages/estimation/proto/estimator_options.pb.h"

int main() {
    using estimation::EstimatorOptions;
    EstimatorOptions estimator_options;
    estimator_options.set_frequency(100.0);
    estimator_options.set_propagateuncertainty(false);
    estimator_options.set_publishaddress("tcp://127.0.0.1");
    estimator_options.set_publishport(7100);
    estimator_options.set_subscribeaddress("tcp://127.0.0.1");
    estimator_options.set_subscribeport(7001);

    {
        std::string text;
        google::protobuf::TextFormat::PrintToString(estimator_options, &text);
        std::fstream output("estimator_options.default.pbtxt", std::ios::out);
        output << text;
        output.close();
    }

    using estimation::WheelOdometryOptions;
    WheelOdometryOptions wheel_odometry_options;
    wheel_odometry_options.mutable_base_options()->MergeFrom(estimator_options);
    wheel_odometry_options.set_mode(WheelOdometryOptions::FOUR_WHEEL);
    {
        std::string text;
        google::protobuf::TextFormat::PrintToString(wheel_odometry_options, &text);
        std::fstream output("wheel_odometry_options.default.pbtxt", std::ios::out);
        output << text;
        output.close();
    }
}
