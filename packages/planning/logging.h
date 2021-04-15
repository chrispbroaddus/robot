#pragma once

#include <atomic>
#include <deque>
#include <fstream>
#include <mutex>
#include <thread>

#include "glog/logging.h"

#include "packages/core/proto/geometry.pb.h"
#include "packages/planning/proto/trajectory.pb.h"

constexpr size_t kMaxTemplateSize = 1024;

namespace planning {

core::SystemTimestamp getTime();

class FilenameCreator {
public:
    explicit FilenameCreator(const std::string& stub)
        : m_created_files({})
        , m_template("") {
        CHECK(!stub.empty());
        std::stringstream template_creator;
        template_creator << "/tmp/" << stub << "_XXXXXX";
        m_template = template_creator.str();
        CHECK(m_template.size() < kMaxTemplateSize);
    }

    std::string operator()() const {
        CHECK(!m_template.empty());
        char mutable_template[kMaxTemplateSize];
        memset(mutable_template, '\0', sizeof(char) * kMaxTemplateSize);
        std::copy(m_template.begin(), m_template.end(), mutable_template);
        int descriptor = mkstemp(mutable_template);
        if (descriptor < 0) {
            LOG(INFO) << "Descriptor: " << descriptor;
            LOG(INFO) << "Template: " << std::string(m_template);
            LOG(INFO) << "Target: " << std::string(mutable_template);
            LOG(FATAL) << "Cannot continue.";
        }
        m_created_files.push_back(mutable_template);
        return mutable_template;
    }

    ~FilenameCreator() {
        for (const auto& file : m_created_files) {
            CHECK(remove(file.c_str()) == 0);
        }
    }

private:
    mutable std::deque<std::string> m_created_files;
    std::string m_template;
};

template <class T> bool readLog(const std::string& logname, std::deque<T>& entries);

template <class T> class BaseLogger {

public:
    BaseLogger(const std::string& log);

    bool add(const T& entry);

    virtual ~BaseLogger();

protected:
    void run();
    std::atomic<bool> m_continue;
    std::mutex m_lock;
    std::ofstream m_log;
    std::thread m_writer_thread;
    std::atomic<int64_t> m_written;
    std::atomic<int64_t> m_received;
    std::deque<T> m_queue;
};

#include "logging.hpp"

} // planning
