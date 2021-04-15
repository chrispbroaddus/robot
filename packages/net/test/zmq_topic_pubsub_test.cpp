#include "gtest/gtest.h"

#include "packages/core/test/common.h"
#include "packages/net/include/zmq_topic_pub.h"
#include "packages/net/include/zmq_topic_sub.h"

#include "packages/hald/proto/service_list.pb.h"

#include <thread>

using namespace net;

TEST(PubSubTest, interoperabilityTest) {
    constexpr int highWaterMark = 100;
    constexpr int lingerPeriodInMilliseconds = 1000;

    zmq::context_t context = zmq::context_t(1);

    using core::test::UniquePortProvider;
    UniquePortProvider provider;
    const int port = provider.next_port();

    ZMQProtobufPublisher<hald::ServiceList> pub(context, "tcp://*:" + std::to_string(port), highWaterMark, lingerPeriodInMilliseconds);
    ZMQProtobufSubscriber<hald::ServiceList> sub(context, "tcp://localhost:" + std::to_string(port), "services", highWaterMark);

    // Pub/Sub are not synchronized, so here we need to wait a bit until the Sub has had time to start.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_FALSE(sub.poll());

    hald::ServiceList expectedServiceList;
    expectedServiceList.add_topics("topic1");
    expectedServiceList.add_topics("topic2");
    EXPECT_TRUE(pub.send(expectedServiceList, "services"));

    EXPECT_TRUE(sub.poll(std::chrono::milliseconds(100)));

    hald::ServiceList actualServiceList;
    EXPECT_TRUE(sub.recv(actualServiceList));

    EXPECT_EQ(expectedServiceList.topics_size(), actualServiceList.topics_size());
    EXPECT_EQ(expectedServiceList.topics(0), actualServiceList.topics(0));
    EXPECT_EQ(expectedServiceList.topics(1), actualServiceList.topics(1));
}
