
#pragma once

#include "filter.h"
#include "glog/logging.h"
#include "packages/core/include/wait_queue.h"
#include <memory>
#include <thread>

namespace filter_graph {

class SourceFilterThreadRunner {
public:
    typedef core::WaitQueue<std::shared_ptr<Filter> > queue_t;

    SourceFilterThreadRunner(const size_t maxQueueSize)
        : m_stopped(false) {
        m_queue = std::unique_ptr<queue_t>(new queue_t(maxQueueSize));
    }
    ~SourceFilterThreadRunner() = default;

    ///
    /// @return Pointer to the task queue.
    ///
    queue_t* getQueue() { return m_queue.get(); }

    ///
    /// @brief Notify the threads to stop.
    ///
    void stop() { m_stopped = true; }

    ///
    /// @brief Thread service function which is called by the std::thread.
    ///
    void invoke() {
        while (!m_stopped) {
            std::shared_ptr<Filter> filter;
            if (m_queue->dequeue(filter)) {
                filter->invoke();
                m_queue->enqueue(filter);
            }

            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }

private:
    bool m_stopped;
    std::unique_ptr<queue_t> m_queue;
};

class FilterThreadRunner {
public:
    typedef core::WaitQueue<std::shared_ptr<Filter> > queue_t;

    FilterThreadRunner(const size_t maxQueueSize)
        : m_stopped(false) {
        m_queue = std::unique_ptr<queue_t>(new queue_t(maxQueueSize));
    }
    ~FilterThreadRunner() = default;

    ///
    /// @return Pointer to the task queue.
    ///
    queue_t* getQueue() { return m_queue.get(); }

    ///
    /// Notify the threads to stop.
    ///
    void stop() { m_stopped = true; }

    ///
    /// Thread service function which is called by the std::thread.
    ///
    void invoke() {
        while (!m_stopped) {
            std::shared_ptr<Filter> filter;
            if (m_queue->dequeue(filter, std::chrono::microseconds(500))) {
                filter->invoke();
            }
        }
    }

private:
    bool m_stopped;
    std::unique_ptr<queue_t> m_queue;
};

template <typename THREAD_RUNNER_T> class ThreadPool {
public:
    typedef THREAD_RUNNER_T thread_runner_t;
    typedef core::WaitQueue<std::shared_ptr<Filter> > queue_t;

    ThreadPool(const size_t numThreads)
        : m_numThreads(numThreads) {}
    ~ThreadPool() = default;

    ///
    /// Attached a downstream filter which will be notified when a container is available
    /// for processing.
    ///
    void attachedFilter(std::shared_ptr<Filter> filter) { m_filters.push_back(filter); }

    ///
    /// Initialize the thread pool. This function must be called before start().
    ///
    void initialize() { m_threadRunner = std::unique_ptr<thread_runner_t>(new thread_runner_t(m_filters.size())); }

    ///
    /// Start the threads in the pool and wait notify the filters.
    ///
    void start() {

        LOG(INFO) << "Creating " << m_filters.size() << " threads";

        for (auto& filter : m_filters) {
            m_threadRunner->getQueue()->enqueue(filter);
        }

        for (size_t i = 0; i < m_numThreads; i++) {
            m_threads.push_back(std::thread(&ThreadPool::invoke, this));
        }
    }

    ///
    /// Stops the threads and blocks until all the threads can be joined.
    ///
    void stop() {
        m_threadRunner->stop();
        for (auto& t : m_threads) {
            t.join();
        }
    }

    ///
    /// Get a pointer to the thread runner.
    ///
    thread_runner_t* getThreadRunner() {
        assert(m_threadRunner.get());
        return m_threadRunner.get();
    }

private:
    const size_t m_numThreads;
    std::vector<std::thread> m_threads;
    std::vector<std::shared_ptr<Filter> > m_filters;
    std::unique_ptr<thread_runner_t> m_threadRunner;

    void invoke() { m_threadRunner->invoke(); }
};
}
