
#pragma once

#include "filter.h"

namespace filter_graph {

class TransformFilter : public Filter {
public:
    TransformFilter(const std::string& filterName, const size_t inputQueueSize, const size_t outputQueueSize)
        : Filter(filterName) {
        m_inputQueue = std::unique_ptr<Filter::queue_t>(new Filter::queue_t(inputQueueSize));
        m_outputQueue = std::unique_ptr<Filter::queue_t>(new Filter::queue_t(outputQueueSize));
    }
    ~TransformFilter() = default;

    ///
    /// @brief Receive a container from an upstream filter. This function will be
    /// called on a thread in the thread pool.
    ///
    virtual void receive(std::shared_ptr<Container> container) = 0;

protected:
    ///
    /// @brief Callback from thread pool.
    ///
    void invoke() override { processInputQueue(); }

    ///
    /// @brief Processes the input queue containers coming from an upstream filter.
    ///
    void processInputQueue() {

        container_t container;
        while (getInputQueue()->dequeue(container)) {
            receive(container);
        }
    }
};
}
