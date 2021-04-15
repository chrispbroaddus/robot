#include "packages/executor/proto_helpers.h"

#include "glog/logging.h"

#include "packages/planning/proto_helpers.h"

namespace executor {

constexpr char kDefaultExecutorOptions[] = "config/global/executor_options.default.pbtxt";

ExecutorOptions loadDefaultExecutorOptions() {
    ExecutorOptions options;
    std::string serialized_proto = planning::loadProtoText(kDefaultExecutorOptions);
    CHECK(google::protobuf::TextFormat::MergeFromString(serialized_proto, &options));
    return options;
}

} // executor
