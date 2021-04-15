#pragma once

#include <chrono>
#include <zmq.hpp>

#include "glog/logging.h"

namespace net {

/// A zeromq based subscriber class that is templated on the type of protobuf messages that it receives
/// It receives messages sent by a zeromq publisher
template <typename PROTO_MESSAGE_T> class ZMQProtobufSubscriber {
public:
    ZMQProtobufSubscriber() = delete;
    ZMQProtobufSubscriber(const ZMQProtobufSubscriber& other) = delete;
    ZMQProtobufSubscriber(zmq::context_t& context, const std::string& serverAddress, const std::string& topic, const int highWaterMark)
        : m_subSocket(context, ZMQ_SUB) {
        LOG(INFO) << "connecting to " << serverAddress << ", topic " << topic;
        m_subSocket.setsockopt(ZMQ_RCVHWM, highWaterMark);
        m_subSocket.connect(serverAddress);
        m_subSocket.setsockopt(ZMQ_SUBSCRIBE, topic.c_str(), topic.size());
    }
    ~ZMQProtobufSubscriber() = default;

    /// Poll the socket to determine if there are any messages available with a specified timeout in milliseconds.
    /// @return true if there are items available on the queue
    bool poll(std::chrono::milliseconds timeout) {
        zmq::pollitem_t pollItem[1];
        pollItem[0].socket = m_subSocket;
        pollItem[0].events = ZMQ_POLLIN;
        return zmq::poll(pollItem, 1, static_cast<long>(timeout.count())) > 0;
    }

    /// Poll the socket to determine if there are any messages available with a specified timeout in microseconds.
    /// @return true if there are items available on the queue
    [[deprecated("Zeromq version 4.2 doesn't support timeouts in microseconds. Switch to milliseconds")]] bool poll(
        std::chrono::microseconds timeout) {
        if (timeout.count()) {
            return poll(std::chrono::duration_cast<std::chrono::milliseconds>(timeout));
        } else {
            return poll(std::chrono::milliseconds(0));
        }
    }

    /// Poll the socket to determine if there are any messages available without a timeout. The
    /// function returns immediately.
    /// @return true if there are items available on the queue
    bool poll() { return poll(std::chrono::milliseconds(0)) > 0; }

    /// Receive a message on the socket. If there is no message it will block indefinitely.
    bool recv(PROTO_MESSAGE_T& message) {
        zmq::message_t envelope;
        if (m_subSocket.recv(&envelope) == 0) {
            return false;
        }

        zmq::message_t msg;
        if (m_subSocket.recv(&msg) == 0) {
            return false;
        }

        return message.ParseFromArray(msg.data(), msg.size());
    }

private:
    zmq::socket_t m_subSocket;
};

} // net
