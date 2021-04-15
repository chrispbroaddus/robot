#include "packages/unity_plugins/connections_reader/include/connections_reader.h"
#include "glog/logging.h"
#include "google/protobuf/util/json_util.h"
#include <exception>
#include <fstream>
#include <sstream>

using namespace unity_plugins;

ConnectionsReader::ConnectionsReader(const std::string& filename)
    : m_lastConnection(nullptr) {
    std::ifstream jsonStream = std::ifstream(filename);
    if (!jsonStream.is_open()) {
        throw std::runtime_error("ConnectionsReader: The json stream data is invalid: " + filename);
    }

    std::stringstream buffer;
    buffer << jsonStream.rdbuf();
    auto status = google::protobuf::util::JsonStringToMessage(buffer.str(), &m_connections);

    if (!status.ok()) {
        throw std::runtime_error("ConnectionsReader: The connections file contains invalid data: " + status.ToString() + " - " + filename);
    }
}

const mercury::Connection* ConnectionsReader::findConnection(mercury::topic topic) {
    if (m_lastConnection && m_lastConnection->topic() == topic) {
        return m_lastConnection;
    }

    for (int ii = 0; ii < m_connections.connection_size(); ++ii) {
        const auto connection = &m_connections.connection(ii);
        if (connection->topic() == topic) {
            m_lastConnection = connection;
            return m_lastConnection;
        }
    }

    m_lastConnection = nullptr;
    return nullptr;
}

bool ConnectionsReader::hasConnection(mercury::topic topic) {
    auto connection = findConnection(topic);
    return connection != nullptr;
}

const std::string& ConnectionsReader::address(mercury::topic topic) {
    auto connection = findConnection(topic);
    if (connection) {
        return connection->address();
    }

    static const std::string empty;
    return empty;
}

int ConnectionsReader::port(mercury::topic topic) {
    auto connection = findConnection(topic);
    if (connection) {
        return connection->port();
    }

    return -1;
}

const std::string ConnectionsReader::fullAddress(mercury::topic topic) {
    auto connection = findConnection(topic);
    if (connection) {
        return "tcp://" + connection->address() + ":" + std::to_string(connection->port());
    }

    return "";
}
