#include "packages/estimation/proto_helpers.h"
#include "packages/planning/proto_helpers.h"

#include "glog/logging.h"

namespace estimation {

constexpr char kDefaultEstimatorOptions[] = "config/global/estimator_options.default.pbtxt";

EstimatorOptions loadDefaultEstimatorOptions() {
    EstimatorOptions options;
    std::string serialized_proto = planning::loadProtoText(kDefaultEstimatorOptions);
    CHECK(google::protobuf::TextFormat::MergeFromString(serialized_proto, &options));
    return options;
}

constexpr char kDefaultWheelOdometryEstimatorOptions[] = "config/global/wheel_odometry_options.default.pbtxt";

WheelOdometryOptions loadDefaultWheelOdometryOptions() {
    WheelOdometryOptions options;
    std::string serialized_proto = planning::loadProtoText(kDefaultWheelOdometryEstimatorOptions);
    CHECK(google::protobuf::TextFormat::MergeFromString(serialized_proto, &options));
    return options;
}
} // estimation
