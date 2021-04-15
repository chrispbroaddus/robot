#include "packages/unity_plugins/connections_reader/include/connections_reader.h"
#include "glog/logging.h"
#include "packages/unity_plugins/connections_reader/include/default_simulator_connections.h"
#include "gtest/gtest.h"
#include <fstream>
#include <google/protobuf/util/json_util.h>
#include <string>

using namespace unity_plugins;

TEST(SimulatorSettingsReader, ctor) {

    const std::string testJsonFile("/tmp/connections.json");

    // write out the default settings to a temp file
    mercury::Connections defaultConnections = defaultSimulatorConnections();
    std::string msg;
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    google::protobuf::util::MessageToJsonString(defaultConnections, &msg, options);

    std::ofstream out(testJsonFile);
    out << msg;
    out.close();

    // read in settings
    ConnectionsReader reader(testJsonFile);

    // compare
    EXPECT_EQ(defaultConnections.connection_size(), reader.connections().connection_size());

    for (int ii = 0; ii < defaultConnections.connection_size(); ++ii) {
        const auto& defaultConnection = defaultConnections.connection(ii);
        const auto& topic = defaultConnection.topic();

        // check the message was read in the same
        EXPECT_EQ(defaultConnection.address(), reader.connections().connection(ii).address());
        EXPECT_EQ(defaultConnection.port(), reader.connections().connection(ii).port());
        EXPECT_EQ(defaultConnection.topic(), reader.connections().connection(ii).topic());

        // check our query api
        EXPECT_TRUE(reader.hasConnection(topic));
        EXPECT_EQ(defaultConnection.address(), reader.address(topic));
        EXPECT_EQ(defaultConnection.port(), reader.port(topic));

        // check our helper api
        std::string fullAddress = "tcp://" + defaultConnection.address() + ":" + std::to_string(defaultConnection.port());
        EXPECT_EQ(fullAddress, reader.fullAddress(topic));
    }

    // check false
    auto falseTopic = mercury::topic::execution;
    EXPECT_FALSE(reader.hasConnection(falseTopic));
    EXPECT_EQ("", reader.address(falseTopic));
    EXPECT_EQ(-1, reader.port(falseTopic));
}
