#include "glog/logging.h"

namespace logging {

/// Log sink that pushes messages to syslog
///
/// To make use of this, call google::AddLogSink(...) in your main with an instance of this class as its argument.
class SyslogSink : public google::LogSink {
public:
    SyslogSink();

    // Sink's logging logic (message_len is such as to exclude '\n' at the end).
    // This method can't use LOG() or CHECK() as logging system mutex(s) are held
    // during this call.
    void send(google::LogSeverity severity, const char* full_filename, const char* /* base_filename */, int line,
        const struct ::tm* /* tm_time */, const char* message, size_t message_len) final;
};

/// Convenience method. Call this in your main to have glog also send all messages to syslog.
void useSyslogSink();
}