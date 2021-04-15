
#pragma once

#include <zmq.hpp>

namespace net {

// SendProtobuf sends a topic followed by an encoded protobuf.
template <typename PROTO_MESSAGE_T> bool SendProtobuf(zmq::socket_t& pub, const PROTO_MESSAGE_T& message, const std::string& topic) {
    zmq::message_t filter(topic.size());
    memcpy(filter.data(), topic.c_str(), topic.size());

    zmq::message_t msg(message.ByteSize());
    if (!message.SerializeToArray(msg.data(), msg.size())) {
        return false;
    }

    if (!pub.send(filter, ZMQ_SNDMORE)) {
        return false;
    }
    if (!pub.send(msg)) {
        return false;
    }

    return true;
}

/// A zeromq based publisher class that is templated on the type of protobuf messages that it sends
/// It sends messages that can be received by a zeromq subscriber
template <typename PROTO_MESSAGE_T> class ZMQProtobufPublisher {
public:
    ZMQProtobufPublisher() = delete;
    ZMQProtobufPublisher(const ZMQProtobufPublisher& other) = delete;
    ZMQProtobufPublisher(
        zmq::context_t& context, const std::string& serverAddress, const int highWaterMark, const int lingerPeriodInMilliseconds)
        : m_pubSocket(context, ZMQ_PUB) {
        m_pubSocket.setsockopt(ZMQ_SNDHWM, highWaterMark);
        m_pubSocket.setsockopt(ZMQ_LINGER, lingerPeriodInMilliseconds);
        m_pubSocket.bind(serverAddress);
    }
    ~ZMQProtobufPublisher() = default;

    /// Send a protobuf message with ZMQ on the topic.
    /// @return false if the sending of the message fails, otherwise true
    bool send(const PROTO_MESSAGE_T& message, const std::string& topic) { return SendProtobuf(m_pubSocket, message, topic); }

private:
    zmq::socket_t m_pubSocket;
};

} // net
