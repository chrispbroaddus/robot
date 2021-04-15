
#pragma once

#include <thread>

namespace hald {

class DeviceThread {
public:
    DeviceThread();
    ~DeviceThread();

    /// Start the thread for processing
    void start();

    /// Stop the thread
    void stop();

    /// The function called by the thread
    virtual void run() = 0;

protected:
    bool m_cancel;

private:
    std::unique_ptr<std::thread> m_thread;
};
}