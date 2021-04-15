#pragma once

#include "glog/logging.h"

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <iostream>

namespace serialization {

///
/// @brief ProtobufWriter is a simple abstraction around writing delimited data to an std::ostream.
///
class ProtobufWriter {
public:
    ProtobufWriter(std::ostream* outputStream)
        : m_outputStream(outputStream)
        , m_zeroCopyOutputStream(outputStream)
        , m_codedOutputStream(&m_zeroCopyOutputStream)
        , m_size(0) {}

    template <typename MSG_T> bool writeNext(const MSG_T& msg) {
        m_codedOutputStream.WriteVarint32(msg.ByteSize());
        if (!msg.SerializeToCodedStream(&m_codedOutputStream)) {
            LOG(ERROR) << "error writing protobuf to stream";
            return false;
        }
        m_size += msg.ByteSize();
        return true;
    }

    uint64_t size() const { return m_size; }

private:
    std::ostream* m_outputStream;
    google::protobuf::io::OstreamOutputStream m_zeroCopyOutputStream;
    google::protobuf::io::CodedOutputStream m_codedOutputStream;
    uint64_t m_size;
};

constexpr size_t MAX_FILE_SIZE = 2047 * 1024 * 1024;
constexpr size_t WARNING_THRESHOLD = 1000 * 1024 * 1024;

///
/// @brief ProtobufReader is a simple abstraction around reading delimited data from an std::istream.
///
class ProtobufReader {
public:
    ProtobufReader(std::istream* inputStream) {

        using namespace google::protobuf::io;

        m_zeroCopyInputStream = std::unique_ptr<IstreamInputStream>(new IstreamInputStream(inputStream));
        m_codedInputStream = std::unique_ptr<CodedInputStream>(new CodedInputStream(m_zeroCopyInputStream.get()));
        m_codedInputStream->SetTotalBytesLimit(MAX_FILE_SIZE, WARNING_THRESHOLD);
    }

    template <typename MSG_T> bool readNext(MSG_T& msg) {
        uint32_t byteSize;
        if (!m_codedInputStream->ReadVarint32(&byteSize)) {
            LOG(ERROR) << "error reading varint32 from protobuf stream";
            return false;
        }

        auto limit = m_codedInputStream->PushLimit(byteSize);
        if (!msg.ParseFromCodedStream(m_codedInputStream.get())) {
            LOG(ERROR) << "error decoding protobuf from stream";
            return false;
        }
        m_codedInputStream->PopLimit(limit);
        return true;
    }

private:
    std::unique_ptr<google::protobuf::io::ZeroCopyInputStream> m_zeroCopyInputStream;
    std::unique_ptr<google::protobuf::io::CodedInputStream> m_codedInputStream;
};
}
