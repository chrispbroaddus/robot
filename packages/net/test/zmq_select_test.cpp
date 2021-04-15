
#include "glog/logging.h"
#include "gtest/gtest.h"

#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/net/include/zmq_select.h"
#include "packages/net/include/zmq_topic_pub.h"

#include <thread>

using namespace net;

TEST(SelectTest, OneSocket) {
    zmq::context_t ctx;

    // create an in-process publisher for testing purposes
    zmq::socket_t pub(ctx, ZMQ_PAIR);
    pub.bind("inproc://SelectTest");

    // create an in-process subscriber for testing purposes
    zmq::socket_t sub(ctx, ZMQ_PAIR);
    sub.connect("inproc://SelectTest");

    // send a mock camera sample
    hal::CameraSample src;
    src.set_id(123);
    SendProtobuf(pub, src, "camera");

    // run the select loop
    ZMQSelectLoop select;
    hal::CameraSample dest;
    select.OnProtobuf<hal::CameraSample>(sub, "camera", [&](const hal::CameraSample& sample) { dest = sample; });

    select.Poll();

    // check the received protobuf
    EXPECT_EQ(123, dest.id());
}

TEST(SelectTest, TwoSockets) {
    zmq::context_t ctx;

    // create an in-process publisher for testing purposes
    zmq::socket_t pub1(ctx, ZMQ_PAIR);
    pub1.bind("inproc://SelectTest1");

    // create an in-process subscriber for testing purposes
    zmq::socket_t sub1(ctx, ZMQ_PAIR);
    sub1.connect("inproc://SelectTest1");

    // create an in-process publisher for testing purposes
    zmq::socket_t pub2(ctx, ZMQ_PAIR);
    pub2.bind("inproc://SelectTest2");

    // create an in-process subscriber for testing purposes
    zmq::socket_t sub2(ctx, ZMQ_PAIR);
    sub2.connect("inproc://SelectTest2");

    // send a mock camera sample
    hal::CameraSample src;
    src.set_id(123);
    SendProtobuf(pub2, src, "camera");

    hal::CameraSample dest1;
    hal::CameraSample dest2;

    // run the select loop
    ZMQSelectLoop select;
    select.OnProtobuf<hal::CameraSample>(sub1, "camera", [&](const hal::CameraSample& sample) { dest1 = sample; });
    select.OnProtobuf<hal::CameraSample>(sub2, "camera", [&](const hal::CameraSample& sample) { dest2 = sample; });

    select.Poll();

    // check the received protobuf
    EXPECT_EQ(0, dest1.id());
    EXPECT_EQ(123, dest2.id());
}
