#include "glog/logging.h"

#include <fstream>
#include <mutex>

namespace perception {

template <typename MESSAGE_TYPE> class LogWriter {
public:
    explicit LogWriter(const std::string filename)
        : m_filename(filename) {
        CHECK(!filename.empty());
        LOG(INFO) << "Logging to: " << filename;
        m_output_stream.open(m_filename, std::ios::binary);
        CHECK(m_output_stream.good());
    }

    bool write(const MESSAGE_TYPE& message) {
        std::string serialized_message;
        CHECK(message.SerializeToString(&serialized_message));
        int32_t size = serialized_message.size();
        std::lock_guard<std::mutex> lock(m_guard);
        m_output_stream.write(reinterpret_cast<const char*>(&size), sizeof(int32_t));
        m_output_stream.write(serialized_message.c_str(), serialized_message.size());
        return m_output_stream.good();
    }
    ~LogWriter() { m_output_stream.close(); }

private:
    const std::string m_filename;
    std::ofstream m_output_stream;
    std::mutex m_guard;
};

template <typename MESSAGE_TYPE> class LogReader {
public:
    explicit LogReader(const std::string filename)
        : m_filename(filename) {
        CHECK(!filename.empty());
        LOG(INFO) << "Reading from: " << filename;
        m_input_stream.open(m_filename, std::ios::binary);
        CHECK(m_input_stream.good());
    }

    bool read(MESSAGE_TYPE& message) {
        std::lock_guard<std::mutex> lock(m_guard);
        CHECK(m_input_stream.good());
        int32_t size = 0;
        m_input_stream.read(reinterpret_cast<char*>(&size), sizeof(int32_t));
        if (m_input_stream.eof()) {
            return false;
        }
        std::string payload;
        payload.resize(size);
        m_input_stream.read(const_cast<char*>(payload.c_str()), size);
        CHECK(message.ParseFromString(payload));
        return true;
    }

    ~LogReader() { m_input_stream.close(); }

private:
    const std::string m_filename;
    std::ifstream m_input_stream;
    std::mutex m_guard;
};

} // perception
