#include "gtest/gtest.h"

#include "packages/core/test/common.h"
#include "packages/estimation/estimator.h"
#include "packages/estimation/proto/estimator_options.pb.h"

namespace {
constexpr int kDefaultQueueLength = 1000;
}

TEST(estimator, configuration) {
    using estimation::EstimatorOptions;
    using estimation::loadDefaultEstimatorOptions;
    { EstimatorOptions options = loadDefaultEstimatorOptions(); }
}

TEST(estimator, instantiate) {
    using estimation::OdometryEstimator;
    using estimation::EstimatorOptions;
    EstimatorOptions options;
    options.set_subscribeaddress("tcp://127.0.0.1");
    core::test::UniquePortProvider provider;
    options.set_subscribeport(provider.next_port());
    options.set_publishaddress("tcp://127.0.0.1");
    options.set_publishport(provider.next_port());
    options.set_frequency(100);
    options.set_propagateuncertainty(false);
    {
        std::unique_ptr<OdometryEstimator> estimator_ptr;
        estimator_ptr.reset(new OdometryEstimator(options));
        ASSERT_NE(estimator_ptr.get(), nullptr);
    }
}

namespace estimation {
namespace test {
    class DummyEstimator : public OdometryEstimator {
    public:
        DummyEstimator(EstimatorOptions& options)
            : OdometryEstimator(options) {}
    };
} // test
} // estimation

namespace estimation {
class OdometryGetterTester : public Estimator {
public:
    OdometryGetterTester(const int num_poses) {
        for (int i = 0; i < num_poses; ++i) {
            core::SystemTimestamp stamp;
            stamp.set_nanos(i);
            State state;
            initialiseState(state);
            state.m_pose.translation()[0] = 1;
            state.m_time = std::chrono::nanoseconds(stamp.nanos());
            m_stateHistory.emplace_back(state);
        }
    }
};
} // estimation

TEST(estimator, query) {
    estimation::OdometryGetterTester tester(kDefaultQueueLength);
    const auto& states = tester.states();
    ASSERT_TRUE(states.size() == kDefaultQueueLength);
    core::SystemTimestamp query;
    query.set_nanos(-2);
    estimation::State result;
    ASSERT_FALSE(tester.get(query, result));
    query.set_nanos(1);
    ASSERT_TRUE(tester.get(query, result));
    ASSERT_EQ(result.m_pose.translation()[0], 1);
    query.set_nanos(kDefaultQueueLength + 2);
    ASSERT_FALSE(tester.get(query, result));
}

TEST(estimator, wheelOdometryUpdate) {
    using estimation::WheelOdometryEstimator;
    using estimation::WheelOdometryOptions;
    using estimation::EstimatorOptions;
    std::unique_ptr<WheelOdometryEstimator> odometry;

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
    odometry.reset(new WheelOdometryEstimator(options));
    ASSERT_NE(odometry.get(), nullptr);
    int count = 1;
    for (int i = 0; i < 100; ++i) {
        hal::VCUTelemetryEnvelope envelope;
        envelope.mutable_sendtimestamp()->set_nanos(count);
        hal::VCUWheelEncoderTelemetry telemetry;
        telemetry.mutable_periodstarthardwaretimestamp()->set_nanos(count);
        telemetry.mutable_periodendhardwaretimestamp()->set_nanos(count + 10);
        *envelope.mutable_wheelencoder() = telemetry;
        odometry->update(envelope);
        count += 10;
    }
}

TEST(estimator, wheelOdometryPropagation) {
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

    hal::VCUTelemetryEnvelope envelope;
    envelope.mutable_sendtimestamp()->set_nanos(0);

    hal::VCUServoTelemetry steer_telemetry;
    constexpr hal::VCUServoID kDefaultServoID = hal::VCUServoID::LeftFrontServo;
    steer_telemetry.set_servoid(kDefaultServoID);
    steer_telemetry.set_servoangleradians(M_PI / 6);
    *envelope.mutable_servo() = steer_telemetry;
    odometry->update(envelope);

    constexpr int kDefaultIntervalNanoseconds = 1e8; // 100 ms
    constexpr hal::VCUWheelID kDefaultWheelID = hal::VCUWheelID::LeftFrontWheel;
    hal::VCUWheelEncoderTelemetry wheel_telemetry;
    wheel_telemetry.mutable_periodstarthardwaretimestamp()->set_nanos(0);
    wheel_telemetry.mutable_periodendhardwaretimestamp()->set_nanos(kDefaultIntervalNanoseconds);
    wheel_telemetry.set_wheelid(kDefaultWheelID);
    wheel_telemetry.set_lineardisplacementmeters(0.1); // Results in 1 m/s
    *envelope.mutable_wheelencoder() = wheel_telemetry;
    odometry->update(envelope);

    estimation::State current_state;
    for (int i = 0; i < 125; ++i) {
        current_state = odometry->states().front();
    }
    const auto translation = current_state.m_pose.translation();
    ASSERT_LT(std::abs(translation[0]), 0.1);
    ASSERT_LT(std::abs(translation[1]), 0.1);
}

TEST(estimator, erroneousTelemetryData) {
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

    hal::VCUTelemetryEnvelope envelope;
    envelope.mutable_sendtimestamp()->set_nanos(0);

    hal::VCUServoTelemetry steer_telemetry;
    constexpr hal::VCUServoID kDefaultServoID = hal::VCUServoID::LeftFrontServo;

    steer_telemetry.set_servoid(kDefaultServoID);
    steer_telemetry.set_servoangleradians(M_PI / 6);
    *envelope.mutable_servo() = steer_telemetry;
    odometry->update(envelope);

    constexpr hal::VCUWheelID kDefaultWheelID = hal::VCUWheelID::LeftFrontWheel;
    hal::VCUWheelEncoderTelemetry wheel_telemetry;

    wheel_telemetry.set_wheelid(kDefaultWheelID);
    wheel_telemetry.mutable_periodstarthardwaretimestamp()->set_nanos(0);
    wheel_telemetry.mutable_periodendhardwaretimestamp()->set_nanos(0);
    wheel_telemetry.set_lineardisplacementmeters(0.1); // Results in 1 m/s
    *envelope.mutable_wheelencoder() = wheel_telemetry;
    ASSERT_THROW(odometry->update(envelope), std::runtime_error);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
