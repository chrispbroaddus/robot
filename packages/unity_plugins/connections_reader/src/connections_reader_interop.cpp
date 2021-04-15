#include "packages/unity_plugins/connections_reader/include/connections_reader.h"
#include "glog/logging.h"
#include <memory>

using namespace unity_plugins;

static std::unique_ptr<ConnectionsReader> s_reader;

extern "C" {
///
/// \brief Load the connections from a single json file to construct ConnectionsReader
/// \return 1 : Success, 0 : Fail
///
int ConnectionsReader_load(const char* filename) {
    std::string filenameStr(filename);
    try {
        s_reader.reset(new ConnectionsReader(filenameStr));
        return 1;
    } catch (const std::exception& e) {
        LOG(ERROR) << e.what();
        return 0;
    }
}

///
/// \brief Check if a connection exists
/// \return 1 : True, 0 : False
///
int ConnectionsReader_hasConnection(mercury::topic topic) {
    if (s_reader) {
        return s_reader->hasConnection(topic) ? 1 : 0;
    }

    return 0;
}

///
/// \brief Get the address for a topic
/// \return address or null
///
const char* ConnectionsReader_address(mercury::topic topic) {
    if (s_reader) {
        auto address = s_reader->address(topic);
        return strdup(address.c_str());
    }

    return nullptr;
}

///
/// \brief Get the port for a topic
/// \return port number or -1
///
int ConnectionsReader_port(mercury::topic topic) {
    if (s_reader) {
        return s_reader->port(topic);
    }

    return -1;
}

///
/// \brief Get the full address for a topic
/// \return full address or null;
///
const char* ConnectionsReader_fullAddress(mercury::topic topic) {
    if (s_reader) {
        return strdup(s_reader->fullAddress(topic).c_str());
    }

    return nullptr;
}
}
