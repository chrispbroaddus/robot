#pragma once

#include <chrono>
#include <zmq.hpp>

namespace net {

/// A zeromq based Reply Server that is based on zeromq's Request-Reply model
/// The class is templated on the type of protobuf messages it can send and receive
/// It receives requests as a protobuf message from the client and sends another protobuf message in response
template <typename PROTO_REQ_MESSAGE_T, typename PROTO_REP_MESSAGE_T> class ZMQProtobufRepServer {
public:
    ZMQProtobufRepServer() = delete;
    ZMQProtobufRepServer(const ZMQProtobufRepServer& other) = delete;
    ZMQProtobufRepServer(zmq::context_t& context, const std::string& bindAddress, const int lingerPeriodInMilliseconds,
        const int sendRecvTimeoutInMilliseconds)
        : m_repSocket(context, ZMQ_REP) {

        m_repSocket.setsockopt(ZMQ_LINGER, lingerPeriodInMilliseconds);
        m_repSocket.setsockopt(ZMQ_RCVTIMEO, sendRecvTimeoutInMilliseconds);
        m_repSocket.setsockopt(ZMQ_SNDTIMEO, sendRecvTimeoutInMilliseconds);
        m_repSocket.bind(bindAddress);
    }
    ~ZMQProtobufRepServer() = default;

    /// Receive a request protobuf message from the client with ZMQ
    /// @return false if receive fails, otherwise true
    bool recv(PROTO_REQ_MESSAGE_T& message) {

        zmq::message_t msg;
        if (!m_repSocket.recv(&msg)) {
            return false;
        }

        message.ParseFromArray(msg.data(), msg.size());

        return true;
    }

    /// Send a response protobuf message to the client with ZMQ
    /// @return false if the sending of the message fails, otherwise true
    bool send(const PROTO_REP_MESSAGE_T& message) {

        zmq::message_t msg(message.ByteSize());
        if (!message.SerializeToArray(msg.data(), msg.size())) {
            return false;
        }

        return m_repSocket.send(msg);
    }

private:
    zmq::socket_t m_repSocket;
};

} // net
