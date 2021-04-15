
#include "packages/filter_graph/include/filter.h"
#include "glog/logging.h"
#include "packages/filter_graph/include/thread_pool.h"

using namespace filter_graph;

void Filter::send() {

    if (m_threadRunner == nullptr) {

        Filter::container_t container;
        while (getOutputQueue()->dequeue(container)) {
            for (auto& filter : m_downstreamFilters) {
                filter->getInputQueue()->enqueue(container);
                filter->invoke();
            }
        }

    } else {

        Filter::container_t container;
        while (getOutputQueue()->dequeue(container)) {
            for (auto& filter : m_downstreamFilters) {

                if (m_blockingQueues) {
                    if (!filter->getInputQueue()->enqueue(container, std::chrono::seconds(60))) {
                        LOG(ERROR) << "Failed to enqueue container on filter: " << filter->name();
                    }
                } else {
                    filter->getInputQueue()->enqueue(container);
                }

                if (!m_threadRunner->getQueue()->enqueue(filter, std::chrono::milliseconds(10))) {
                    LOG(ERROR) << "Failed to schedule filter: " << filter->name();
                }
            }
        }
    }
}
