#pragma once

#include "sample.h"

namespace filter_graph {

template <typename T> class AggregateSample : public Sample {
public:
    AggregateSample(const std::string& streamId)
        : Sample(streamId) {}
    ~AggregateSample() = default;

    T& data() { return m_data; }
    const T data() const { return m_data; }

private:
    T m_data;
};
}
