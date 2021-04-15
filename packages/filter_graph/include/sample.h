
#pragma once

#include <string>

namespace filter_graph {

class Sample {
public:
    Sample() = delete;
    Sample(const std::string& streamId)
        : m_streamId(streamId) {}
    ~Sample() = default;

    const std::string& streamId() const { return m_streamId; }

private:
    std::string m_streamId;
};
}
