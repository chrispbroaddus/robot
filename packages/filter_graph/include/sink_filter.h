
#pragma once

#include "filter.h"

namespace filter_graph {

class SinkFilter : public Filter {
public:
    SinkFilter(const std::string& filterName, const size_t inputQueueSize)
        : Filter(filterName) {
        m_inputQueue = std::unique_ptr<Filter::queue_t>(new Filter::queue_t(inputQueueSize));
    }
    ~SinkFilter() = default;

    ///
    /// @brief Receive a container from from the input queue. The receive function
    /// is called from a thread from the thread pool.
    ///
    virtual void receive(std::shared_ptr<Container> container) = 0;

private:
    ///
    /// @brief Callback from thread pool.
    ///
    void invoke() override { processInputQueue(); }

    ///
    /// @brief Processes the input queue containers coming from an upstream filter.
    ///
    void processInputQueue() {

        Filter::container_t container;
        while (getInputQueue()->dequeue(container)) {
            receive(container);
        }
    }

    ///
    /// @brief Sink filters do not have an output queue, so we override the
    /// output queue accessor function.
    ///
    queue_t* getOutputQueue() override { throw std::runtime_error("sink filters do not have an output queue"); }

    ///
    /// @brief Sink filters do not send containers.
    ///
    void send() override { throw std::runtime_error("sink filters do not send containers"); }
};
}
