
#pragma once

#include <cassert>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <vector>

namespace core {

template <typename VALUE_T> class WaitQueue {
public:
    typedef VALUE_T value_t;

    WaitQueue() = delete;

    WaitQueue(const size_t maxSize)
        : m_ringBuffer(maxSize)
        , m_readIndex(0)
        , m_writeIndex(0)
        , m_numItems(0) {
        assert(maxSize > 0);
    }
    ~WaitQueue() = default;

    // non-copyable
    WaitQueue(const WaitQueue<value_t>&) = delete;
    WaitQueue<value_t>& operator=(const WaitQueue<value_t>&) = delete;

    void enqueue(value_t& item) {
        std::unique_lock<std::mutex> queueGuard(m_mutex);

        if (m_numItems == m_ringBuffer.size()) {
            value_t tmpItem;
            dequeueUnsafe(tmpItem);
        }

        enqueueUnsafe(item);
        m_itemsAvailable.notify_one();
    }

    template <typename Rep, typename Period> bool enqueue(value_t& item, const std::chrono::duration<Rep, Period>& sleep_duration) {
        std::unique_lock<std::mutex> queueGuard(m_mutex);

        if (!m_spaceAvailable.wait_for(queueGuard, sleep_duration, [=] { return m_numItems < m_ringBuffer.size(); })) {
            return false;
        }

        enqueueUnsafe(item);
        m_itemsAvailable.notify_one();

        return true;
    }

    bool dequeue(value_t& output) {
        std::unique_lock<std::mutex> queueGuard(m_mutex);

        if (m_numItems == 0) {
            return false;
        }

        dequeueUnsafe(output);
        m_spaceAvailable.notify_one();

        return true;
    }

    template <typename Rep, typename Period> bool dequeue(value_t& output, const std::chrono::duration<Rep, Period>& sleep_duration) {
        std::unique_lock<std::mutex> queueGuard(m_mutex);

        if (!m_itemsAvailable.wait_for(queueGuard, sleep_duration, [=] { return m_numItems > 0; })) {
            return false;
        }

        dequeueUnsafe(output);
        m_spaceAvailable.notify_one();

        return true;
    }

    bool empty() const {
        std::unique_lock<std::mutex> guard(m_mutex);
        return m_numItems == 0;
    }

private:
    mutable std::mutex m_mutex;

    std::condition_variable m_spaceAvailable;
    std::condition_variable m_itemsAvailable;

    std::vector<value_t> m_ringBuffer;

    size_t m_readIndex;
    size_t m_writeIndex;
    size_t m_numItems;

    void enqueueUnsafe(const value_t& item) {
        ++m_numItems;
        m_ringBuffer[m_writeIndex] = item;
        m_writeIndex = (m_writeIndex + 1) % m_ringBuffer.size();
    }

    void dequeueUnsafe(value_t& item) {
        --m_numItems;
        item = m_ringBuffer[m_readIndex];
        m_readIndex = (m_readIndex + 1) % m_ringBuffer.size();
    }
};
}
