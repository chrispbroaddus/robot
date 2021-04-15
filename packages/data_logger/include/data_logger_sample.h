#pragma once

#include "packages/filter_graph/include/sample.h"
#include "packages/filter_graph/include/source_filter.h"

namespace data_logger {
namespace details {

    template <typename MSG_T> class DataloggerSample : public filter_graph::Sample {
    public:
        DataloggerSample(const std::string& streamId)
            : filter_graph::Sample(streamId) {}
        ~DataloggerSample() = default;
        DataloggerSample(const DataloggerSample&) = delete;
        DataloggerSample(const DataloggerSample&&) = delete;
        DataloggerSample& operator=(const DataloggerSample&) = delete;
        DataloggerSample& operator=(const DataloggerSample&&) = delete;

        MSG_T& data() { return m_sensorSample; }
        const MSG_T& data() const { return m_sensorSample; }

    private:
        MSG_T m_sensorSample;
    };
}
}
