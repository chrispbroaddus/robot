#pragma once

#include "glog/logging.h"

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <atomic>
#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include <zmq.hpp>

namespace net {

// ZMQSelectLoop provides a loop that polls N sockets and decodes the
// resulting message as a protocol buffer. Everything here runs on a single
// thread. It is not safe to call member functions on a ZMQSelectLoop from
// different threads.
class ZMQSelectLoop {
public:
    // OnProtobuf registers a socket to poll in the select loop. When a
    // message is received that matches the given topic then the handler will
    // be called (on the thread that called ZMQSelectLoop::Loop).
    template <typename PROTO_T>
    void OnProtobuf(zmq::socket_t& socket, const std::string& topic, std::function<void(const PROTO_T&)> handler) {
        using std::placeholders::_1;
        Item item = { &socket, topic, std::bind(handleMessage<PROTO_T>, handler, _1) };
        m_items.push_back(item);
    }

    // OnTick registers a function to call on every iteration of the select
    // loop (on the thread that called ZMQSelectLoop::Loop).
    inline void OnTick(std::function<void()> handler) { m_tick = handler; }

    // Poll blocks until a message arrives on any socket, then dispatches it
    // to its handler (on the same thread that called this function), then
    // returns.
    bool Poll();

    // Loop processes messages from all registered sockets until the StopLoop function is called
    void Loop();

    // Stops the select loop
    void StopLoop() { m_runLoop = false; }

private:
    // Item is an item in the select loop.
    struct Item {
        zmq::socket_t* socket;
        std::string topic;
        std::function<void(const zmq::message_t&)> handler;
    };

    // handleMessage dispatches messages to handlers
    template <typename PROTO_T> static void handleMessage(std::function<void(const PROTO_T&)> handler, const zmq::message_t& msg) {
        PROTO_T pb;

        std::stringstream buffer;
        buffer.write(static_cast<const char*>(msg.data()), msg.size());

        google::protobuf::io::IstreamInputStream istream(&buffer);
        google::protobuf::io::CodedInputStream cstream(&istream);

        cstream.SetTotalBytesLimit(msg.size() + 1, msg.size() + 1);

        if (!pb.ParseFromCodedStream(&cstream)) {
            LOG(WARNING) << "failed to parse protobuf";
            return;
        }

        handler(pb);
    }

    // the list of items
    std::vector<Item> m_items;

    // the function to call when idle
    std::function<void()> m_tick;

    //
    std::atomic_bool m_runLoop;
};

} // net
