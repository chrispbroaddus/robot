template <class T>
BaseLogger<T>::BaseLogger(const std::string& log)
    : m_continue(true)
    , m_written(0)
    , m_received(0) {
    m_log = std::ofstream(log, std::ios::binary);
    CHECK(m_log.good());
    m_writer_thread = std::thread(&BaseLogger::run, this);
}

template <class T> BaseLogger<T>::~BaseLogger() {
    m_continue = false;
    m_writer_thread.join();
    m_log.close();
    CHECK(m_written == m_received) << m_written << " vs. " << m_received;
}

template <class T> void BaseLogger<T>::run() {
    while (m_continue || !m_queue.empty()) {
        /*
         *This loop will exit after:
         *1. Being instructed to stop (m_continue set to false), and
         *2. The write queue is empty
         */
        CHECK(m_log.good());
        if (m_queue.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        std::deque<T> m_queue_copy;
        {
            std::lock_guard<std::mutex> guard(m_lock);
            m_queue_copy.swap(m_queue);
            CHECK(m_queue.empty());
        }
        CHECK(m_queue_copy.size() > 0);
        for (auto message : m_queue_copy) {
            std::string payload;
            message.SerializeToString(&payload);
            uint32_t payload_size = payload.size();
            CHECK(payload_size > 0) << "Invalid payload!";
            m_log.write(reinterpret_cast<const char*>(&payload_size), sizeof(uint32_t));
            m_log.write(payload.c_str(), payload_size);
            m_written++;
        }
        m_log.flush();
    }
}

template <class T> bool BaseLogger<T>::add(const T& entry) {
    std::lock_guard<std::mutex> guard(m_lock);
    m_queue.emplace_back(entry);
    m_received++;
    return true;
}

template <class T> bool readLog(const std::string& logname, std::deque<T>& entries) {
    CHECK(access(logname.c_str(), F_OK) != -1);
    entries.clear();
    std::ifstream input(logname, std::ios::binary);
    CHECK(input.good());

    do {
        int32_t payload_size = 0;
        std::string payload;
        input.read(reinterpret_cast<char*>(&payload_size), sizeof(payload_size));
        if (payload_size == 0) {
            break;
        }
        payload.resize(payload_size);
        input.read(&payload[0], payload.size());
        T entry;
        CHECK(entry.ParseFromString(payload));
        entries.emplace_back(entry);
    } while (input.good());
    return (!entries.empty());
}
