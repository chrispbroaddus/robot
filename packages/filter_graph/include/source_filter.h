
#pragma once

#include "filter.h"
#include "thread_pool.h"

namespace filter_graph {

class SourceFilter : public Filter {
public:
    SourceFilter(const std::string& filterName, const size_t outputQueueSize)
        : Filter(filterName) {
        m_outputQueue = std::unique_ptr<Filter::queue_t>(new Filter::queue_t(outputQueueSize));
    }
    ~SourceFilter() = default;

    ///
    /// @brief Pure virtual function for derived class to implement for container creation.
    ///
    virtual void create() = 0;

private:
    ///
    /// @brief Callback from thread pool.
    ///
    void invoke() override { create(); }

    ///
    /// @brief Source filters do not have an input queue, so we override
    /// the access to the input queue and throw exception.
    ///
    queue_t* getInputQueue() override { throw std::runtime_error("source filters do no have an input queue"); }
};
}
