#include "gtest/gtest.h"

#include "packages/core/test/common.h"
#include "packages/data_logger/proto/config.pb.h"
#include "packages/estimation/estimator.h"
#include "packages/estimation/proto/estimator_options.pb.h"
#include "packages/filesystem/include/filesystem.h"
#include "packages/hal/proto/vcu_telemetry_envelope.pb.h"
#include "packages/serialization/include/protobuf_io.h"

namespace {
class DataLoader {
public:
    explicit DataLoader(const std::string& dataPath) {
        std::ifstream inputFileStream;
        inputFileStream.open(dataPath, std::ios::in | std::ios::binary);
        if (!inputFileStream.is_open()) {
            throw std::runtime_error("Unable to open protodat file: " + dataPath);
        }

        auto protobufReader = std::unique_ptr<serialization::ProtobufReader>(new serialization::ProtobufReader(&inputFileStream));

        std::unique_ptr<google::protobuf::Message> messagePtr;
        messagePtr.reset(new hal::VCUTelemetryEnvelope);

        m_messages.clear();
        while (protobufReader.get()) {
            if (protobufReader->readNext(*dynamic_cast<hal::VCUTelemetryEnvelope*>(messagePtr.get()))) {
                m_messages.emplace_back(*dynamic_cast<hal::VCUTelemetryEnvelope*>(messagePtr.get()));
            } else {
                break;
            }
        }
        inputFileStream.close();
        CHECK(!m_messages.empty());
    }

    const std::vector<hal::VCUTelemetryEnvelope>& messages() const { return m_messages; }

private:
    std::string m_streamDescription;
    std::vector<hal::VCUTelemetryEnvelope> m_messages;
    const std::vector<std::string> m_fileList;
};
}

/**
 * @brief This tests that a simple message log from z2 is correctly processed
 *        by the estimator.
 *        Reason: The estimator treated Z-stage messages as WheelOdometry
 *        messages due to a logic bug.
 */
TEST(estimator, z2CorrectMessageLogTest) {
    using estimation::EstimatorOptions;
    using estimation::WheelOdometryEstimator;
    using estimation::WheelOdometryOptions;

    EstimatorOptions base_options;
    base_options.set_subscribeaddress("tcp://127.0.0.1");
    core::test::UniquePortProvider provider;
    base_options.set_subscribeport(provider.next_port());
    base_options.set_publishaddress("tcp://127.0.0.1");
    base_options.set_publishport(provider.next_port());
    base_options.set_frequency(100);
    base_options.set_propagateuncertainty(false);

    WheelOdometryOptions options;
    options.mutable_base_options()->MergeFrom(base_options);
    options.set_distancetosentinelwheel(1.0);
    std::unique_ptr<WheelOdometryEstimator> odometry;
    odometry.reset(new WheelOdometryEstimator(options));
    ASSERT_NE(odometry.get(), nullptr);
    auto loader = DataLoader("packages/estimation/test/regression/vcu-2017-08-22T23_00_38_protodat");
    for (const auto& message : loader.messages()) {
        odometry->update(message);
    }
}

/**
 * @brief This tests that corrupted messages from the VCU are processed
 *        by the estimator.
 *        Reason: Some messages show instantaneous velocity of over 100m/s, due
 *        to VCU logic issues.
 */
TEST(estimator, z2CorruptMessageLogTest) {
    using estimation::EstimatorOptions;
    using estimation::WheelOdometryEstimator;
    using estimation::WheelOdometryOptions;

    EstimatorOptions base_options;
    base_options.set_subscribeaddress("tcp://127.0.0.1");
    core::test::UniquePortProvider provider;
    base_options.set_subscribeport(provider.next_port());
    base_options.set_publishaddress("tcp://127.0.0.1");
    base_options.set_publishport(provider.next_port());
    base_options.set_frequency(100);
    base_options.set_propagateuncertainty(false);

    WheelOdometryOptions options;
    options.mutable_base_options()->MergeFrom(base_options);
    options.set_distancetosentinelwheel(1.0);
    std::unique_ptr<WheelOdometryEstimator> odometry;
    odometry.reset(new WheelOdometryEstimator(options));
    ASSERT_NE(odometry.get(), nullptr);
    auto loader = DataLoader("packages/estimation/test/regression/vcu-2017-08-29T19_25_00.protodat");
    for (const auto& message : loader.messages()) {
        odometry->update(message);
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
