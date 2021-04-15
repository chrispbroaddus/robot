#include "packages/net/include/zmq_select.h"

#include "glog/logging.h"

#include <memory.h>

namespace net {

bool ZMQSelectLoop::Poll() {
    std::unique_ptr<zmq::pollitem_t[]> sel(new zmq::pollitem_t[m_items.size()]);

    for (size_t i = 0; i < m_items.size(); i++) {
        sel[i].socket = *m_items[i].socket;
        sel[i].events = ZMQ_POLLIN;
    }

    int rc = zmq::poll(sel.get(), m_items.size(), 10);
    if (rc < 0) {
        LOG(WARNING) << "zmq::poll returned with error: " << zmq_strerror(zmq_errno());
        return false;
    }

    zmq::message_t msg;
    for (size_t i = 0; i < m_items.size(); i++) {
        if (sel[i].revents & ZMQ_POLLIN) {
            // first discard the envelope
            if (!m_items[i].socket->recv(&msg)) {
                LOG(WARNING) << "error receiving envelope";
                continue;
            }

            // now read the main message
            if (!m_items[i].socket->recv(&msg)) {
                LOG(WARNING) << "error receiving message";
                continue;
            }

            m_items[i].handler(msg);
        }
    }

    // call the tick handler
    if (m_tick) {
        m_tick();
    }

    return true;
}

void ZMQSelectLoop::Loop() {
    m_runLoop = true;
    while (m_runLoop) {
        if (!Poll()) {
            break;
        }
    }
}

} // namespace net
