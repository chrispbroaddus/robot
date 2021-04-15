#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>

namespace core {
/// Manage a producer/consumer queue. The semantics are as follows:
///
/// N concurrent threads may call enqueue. These calls block until one of the following conditions is met:
/// -# The queue receives a shut down signal. In this case enqueue will do nothing and return false.
/// -# The queue now has available space for the item. In this case the item is copied into the queue and consumers
///  are notified of the availability of new data, after which the function returns true.
/// .
///
/// M concurrent threads may call dequeue. These calls block until one of the following conditions is met:
/// -# The queue receives a shut down signal. In this case, nothing is removed from the queue and the function returns
/// false.
/// -# An item becomes available in the buffer. In this case the item is copied out, producers are notified of the
///  newly available space in the buffer, and the method returns true.
/// .
///
/// Any thread may call shutdown, which signals to both producers and consumers that they should terminate immediately.
/// We make no attempt to guarantee that the queue is drained.
/// \tparam T Queued item type
template <typename T> class BoundedProducerConsumerBuffer {
public:
    BoundedProducerConsumerBuffer()
        : BoundedProducerConsumerBuffer(100) {}

    explicit BoundedProducerConsumerBuffer(size_t capacity)
        : m_capacity(capacity)
        , m_nextRead(0)
        , m_nextWrite(0)
        , m_used(0)
        , m_continue(true)
        , m_mutex()
        , m_canRead()
        , m_canWrite()
        , m_buffer(capacity) {}

    bool enqueue(const T& item) {
        lock_type guard(m_mutex);

        while (m_used == m_capacity && m_continue) {
            m_canWrite.wait(guard);
        }

        if (m_continue) {
            m_buffer.at(m_nextWrite) = item;
            m_nextWrite = (1 + m_nextWrite) % m_capacity;
            ++m_used;

            m_canRead.notify_one();
        }

        return m_continue;
    }

    bool dequeue(T& item) {
        lock_type guard(m_mutex);

        while (0 == m_used && m_continue) {
            m_canRead.wait(guard);
        }

        if (m_continue) {
            item = m_buffer.at(m_nextRead);
            m_nextRead = (1 + m_nextRead) % m_capacity;
            --m_used;

            m_canWrite.notify_one();
        }

        return m_continue;
    }

    void shutdown() {
        lock_type guard(m_mutex);
        m_continue = false;
        m_canWrite.notify_all();
        m_canRead.notify_all();
    }

private:
    using mutex_type = std::mutex;
    using lock_type = std::unique_lock<mutex_type>;

    const size_t m_capacity;
    size_t m_nextRead;
    size_t m_nextWrite;
    size_t m_used;
    std::atomic_bool m_continue;
    mutex_type m_mutex;
    std::condition_variable m_canRead;
    std::condition_variable m_canWrite;
    std::vector<T> m_buffer;
};

/// Specialization that attempts to be smarter about how shared pointers are handled. The interface semantics are identical,
/// the only difference here is that we ensure that before returning a dequeued shared pointer, we explicitly release our
/// references to it.
/// \tparam T
template <typename T> class BoundedProducerConsumerBuffer<std::shared_ptr<T> > {
public:
    BoundedProducerConsumerBuffer()
        : BoundedProducerConsumerBuffer(100) {}

    explicit BoundedProducerConsumerBuffer(size_t capacity)
        : m_capacity(capacity)
        , m_nextRead(0)
        , m_nextWrite(0)
        , m_used(0)
        , m_continue(true)
        , m_mutex()
        , m_canRead()
        , m_canWrite()
        , m_buffer(capacity) {}

    bool enqueue(std::shared_ptr<T> item) {
        lock_type guard(m_mutex);

        while (m_used == m_capacity && m_continue) {
            m_canWrite.wait(guard);
        }

        if (m_continue) {
            m_buffer.at(m_nextWrite) = item;
            m_nextWrite = (1 + m_nextWrite) % m_capacity;
            ++m_used;

            m_canRead.notify_one();
        }

        return m_continue;
    }

    bool dequeue(std::shared_ptr<T>& item) {
        lock_type guard(m_mutex);

        while (0 == m_used && m_continue) {
            m_canRead.wait(guard);
        }

        if (m_continue) {
            // Explicitly release our reference so that it can be cleaned up properly
            item.reset();
            std::swap(item, m_buffer.at(m_nextRead));

            m_nextRead = (1 + m_nextRead) % m_capacity;
            --m_used;

            m_canWrite.notify_one();
        }

        return m_continue;
    }

    void shutdown() {
        lock_type guard(m_mutex);
        for (auto& item : m_buffer) {
            item.reset();
        }

        m_continue = false;
        m_canWrite.notify_all();
        m_canRead.notify_all();
    }

private:
    using mutex_type = std::mutex;
    using lock_type = std::unique_lock<mutex_type>;

    const size_t m_capacity;
    size_t m_nextRead;
    size_t m_nextWrite;
    size_t m_used;
    std::atomic_bool m_continue;
    mutex_type m_mutex;
    std::condition_variable m_canRead;
    std::condition_variable m_canWrite;
    std::vector<std::shared_ptr<T> > m_buffer;
};
}