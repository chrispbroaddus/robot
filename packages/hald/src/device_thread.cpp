
#include "packages/hald/include/device_thread.h"

using namespace hald;

DeviceThread::DeviceThread()
    : m_cancel(false) {}

DeviceThread::~DeviceThread() { stop(); }

void DeviceThread::start() { m_thread = std::unique_ptr<std::thread>(new std::thread(&DeviceThread::run, this)); }

void DeviceThread::stop() {
    if (!m_cancel) {
        m_cancel = true;

        if (m_thread) {
            m_thread->join();
        }
    }
}