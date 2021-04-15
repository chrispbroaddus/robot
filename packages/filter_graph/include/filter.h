
#pragma once

#include "container.h"
#include "packages/core/include/wait_queue.h"
#include <string>

namespace filter_graph {

class FilterThreadRunner;

class Filter {
public:
    typedef std::shared_ptr<Container> container_t;
    typedef core::WaitQueue<container_t> queue_t;

    Filter(const std::string& filterName)
        : m_filterName(filterName)
        , m_threadRunner(nullptr)
        , m_blockingQueues(true) {}
    ~Filter() = default;

    ///
    /// Every filter must implement the INVOKE function.
    ///
    virtual void invoke() = 0;

    ///
    /// @return The name of the filter.
    ///
    const std::string& name() const { return m_filterName; }

    ///
    /// Attach a filter downstream which will receive containers from this filter.
    ///
    void attachDownstreamFilter(std::shared_ptr<Filter> filter) { m_downstreamFilters.push_back(filter); }

    ///
    /// Attach a filter to a thread runner.
    ///
    void attachThreadRunner(FilterThreadRunner* threadRunner, const bool blockingQueues) {
        m_threadRunner = threadRunner;
        m_blockingQueues = blockingQueues;
    }

    ///
    /// @return Pointer to the input queue.
    ///
    virtual queue_t* getInputQueue() { return m_inputQueue.get(); }

    ///
    /// @return Pointer to the output queue.
    ///
    virtual queue_t* getOutputQueue() { return m_outputQueue.get(); }

    ///
    /// @brief The send function sends all the containers in the output queue
    /// to downstream filters input queue.
    ///
    virtual void send();

protected:
    const std::string m_filterName;
    FilterThreadRunner* m_threadRunner;
    bool m_blockingQueues;

    std::unique_ptr<queue_t> m_inputQueue;
    std::unique_ptr<queue_t> m_outputQueue;

    std::vector<std::shared_ptr<Filter> > m_downstreamFilters;
};
}
