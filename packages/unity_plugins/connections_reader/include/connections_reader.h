#pragma once

#include "applications/mercury/proto/connections.pb.h"
#include <map>
#include <string>

namespace unity_plugins {

///
/// Read connections from ProtoBuf-JSON file,
///
class ConnectionsReader {

public:
    ConnectionsReader(const std::string& filename);
    ~ConnectionsReader() = default;
    ConnectionsReader(const ConnectionsReader& obj) = delete;
    ConnectionsReader& operator=(ConnectionsReader other) = delete;

    /// connections in probuffer format
    const mercury::Connections& connections() { return m_connections; }

    /// Check if the specified connection exists
    bool hasConnection(mercury::topic topic);

    /// Get the address for a topic
    const std::string& address(mercury::topic topic);

    /// Get the port for a topic
    int port(mercury::topic topic);

    // Get the full address for a topic (ie tcp://address:port)
    const std::string fullAddress(mercury::topic topic);

private:
    const mercury::Connection* findConnection(mercury::topic topic);

private:
    mercury::Connections m_connections;
    const mercury::Connection* m_lastConnection;
};
} // namespace unity_plugins
