#include "../include/syslog_sink.h"
#include "thirdparty/glog/glog_build/k8/include/glog/logging.h"

#include <mutex>
#include <syslog.h>

namespace {
std::once_flag gInitSyslogFlag;

void initializeSyslog() { ::openlog(NULL, LOG_PID | LOG_CONS | LOG_NDELAY, LOG_USER); }

const char* const kPriorityLabels[] = { "INFO", "WARNING", "ERROR", "FATAL / CRITICAL" };

constexpr const char* kFormat = "%s [%s:%d]: %s";
}

namespace logging {
SyslogSink::SyslogSink() {
    // Guarantee that this executable has made exactly one call to openlog
    std::call_once(gInitSyslogFlag, &initializeSyslog);
}

void SyslogSink::send(google::LogSeverity severity, const char* full_filename, const char* /* base_filename */, int line,
    const struct ::tm* /* tm_time */, const char* message, size_t /* message_len */) {

    int priority = LOG_INFO;
    int priorityLabel = 0;

    switch (severity) {
    case google::GLOG_FATAL:
        priority = LOG_CRIT;
        priorityLabel = 3;
        break;

    case google::GLOG_ERROR:
        priority = LOG_ERR;
        priorityLabel = 2;
        break;

    case google::GLOG_WARNING:
        priority = LOG_WARNING;
        priorityLabel = 1;
        break;

    default:
        // Everything else defaults to info
        break;
    }

    ::syslog(priority, kFormat, kPriorityLabels[priorityLabel], full_filename, line, message);
}

namespace {
    SyslogSink gSyslogSink;
}

void useSyslogSink() { ::google::AddLogSink(&gSyslogSink); }
}