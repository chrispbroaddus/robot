#pragma once

#include <iostream>
#include <zmq.hpp>

namespace net {

/// A zeromq based Request Client that is based on zeromq's Request-Reply model
/// The class is templated on the type of protobuf messages it can send and receive
/// It sends requests as a protobuf message to the server and gets another protobuf message in response
template <typename PROTO_REQ_MESSAGE_T, typename PROTO_REP_MESSAGE_T> class ZMQProtobufReqClient {
public:
    ZMQProtobufReqClient() = delete;
    ZMQProtobufReqClient(const ZMQProtobufReqClient& other) = delete;
    ZMQProtobufReqClient(zmq::context_t& context, const std::string& serverAddress, const int lingerPeriodInMilliseconds,
        const int sendRecvTimeoutInMilliseconds)
        : m_reqSocket(context, ZMQ_REQ) {

        m_reqSocket.setsockopt(ZMQ_LINGER, lingerPeriodInMilliseconds);
        m_reqSocket.setsockopt(ZMQ_RCVTIMEO, sendRecvTimeoutInMilliseconds);
        m_reqSocket.setsockopt(ZMQ_SNDTIMEO, sendRecvTimeoutInMilliseconds);

        /// Relax the requirement that a send must be followed by a receive
        m_reqSocket.setsockopt(ZMQ_REQ_RELAXED, 1);

        /// Makes sure only reply corresponding to the last request is accepted
        m_reqSocket.setsockopt(ZMQ_REQ_CORRELATE, 1);

        m_reqSocket.connect(serverAddress);
    }
    ~ZMQProtobufReqClient() = default;

    /// Send a request protobuf message to the server with ZMQ
    /// @return false if the sending of the message fails, otherwise true
    bool send(const PROTO_REQ_MESSAGE_T& message) {

        zmq::message_t msg(message.ByteSize());
        if (!message.SerializeToArray(msg.data(), msg.size())) {
            return false;
        }

        return m_reqSocket.send(msg);
    }

    /// Receive a response protobuf message from the server with ZMQ
    /// @return false if receive fails, otherwise true
    bool recv(PROTO_REP_MESSAGE_T& message) {

        zmq::message_t msg;
        if (!m_reqSocket.recv(&msg)) {
            return false;
        }

        message.ParseFromArray(msg.data(), msg.size());

        return true;
    }

private:
    zmq::socket_t m_reqSocket;
};

} // net
