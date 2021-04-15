#include <fstream>

#include "glog/logging.h"
#include "google/protobuf/text_format.h"

#include "packages/planning/proto/executor_options.pb.h"

int main() {

    using planning::ExecutorOptions;
    using planning::ExecutionPolicy;
    using planning::CommsType;
    ExecutorOptions executor_options;
    executor_options.set_frequency(1.0);
    executor_options.set_policy(ExecutionPolicy::TIME);
    executor_options.set_comms(CommsType::ZMQ);
    executor_options.set_address("tcp://127.0.0.1");
    executor_options.set_port(5563);
    executor_options.set_require_acknowledge(true);
    executor_options.set_enable_idle(false);
    executor_options.set_logfile("/tmp/executor.log");
    {
        std::string text;
        google::protobuf::TextFormat::PrintToString(executor_options, &text);
        std::fstream output("executor_options.default.pbtxt", std::ios::out);
        output << text;
        output.close();
    }
}
