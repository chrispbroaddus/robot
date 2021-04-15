#include "glog/logging.h"
#include "gtest/gtest.h"

#include "packages/docking/docking_status_publisher.h"
#include "packages/net/include/zmq_topic_sub.h"

#include "packages/core/test/common.h"

#include <atomic>
#include <chrono>
#include <thread>

using core::test::UniquePortProvider;
using namespace docking;

void publish(int portNumber, std::atomic_bool& stop) {

    std::string addr("tcp://*:" + std::to_string(portNumber));
    DockingStatusPublisher publisher(addr, 1, 1000);
    teleop::DockingStatus dockingStatus;
    dockingStatus.set_remaining_distance_x(10);
    dockingStatus.set_remaining_distance_y(5);
    dockingStatus.set_remaining_angle(M_PI / 6);

    while (!stop) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        publisher.publish(dockingStatus);
    }
}

TEST(DockingStatusPublisher, basic_communication) {
    UniquePortProvider provider;
    auto portNumber = provider.next_port();
    std::atomic_bool stop(false);
    std::thread t(publish, portNumber, std::ref(stop));

    zmq::context_t context(1);
    std::string serverAddress("tcp://localhost:" + std::to_string(portNumber));
    std::string topic("docking_status");
    constexpr int highWaterMark = 1;
    net::ZMQProtobufSubscriber<teleop::DockingStatus> subscriber(context, serverAddress, topic, highWaterMark);

    teleop::DockingStatus dockingStatus;
    EXPECT_TRUE(subscriber.poll(std::chrono::milliseconds(1000)));
    subscriber.recv(dockingStatus);
    EXPECT_EQ(dockingStatus.remaining_distance_x(), 10);
    EXPECT_EQ(dockingStatus.remaining_distance_y(), 5);
    EXPECT_NEAR(dockingStatus.remaining_angle(), static_cast<float>(M_PI / 6), std::numeric_limits<float>::epsilon() * 1e2f);
    stop = true;
    t.join();
}
