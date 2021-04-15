#include <sstream>

#include "gtest/gtest.h"

#include "packages/core/proto/timestamp.pb.h"
#include "packages/serialization/include/protobuf_io.h"

#include <fstream>

using namespace serialization;

TEST(Protobuf, WriteReadTest) {
    const size_t numSamplesToWrite = 10;
    std::stringstream buf;

    // enclose the ProtobufWriter in a scope so that the output gets flushed
    // before we try to read it back
    {
        ProtobufWriter protobufWriter(&buf);
        for (size_t i = 0; i < numSamplesToWrite; i++) {
            core::SystemTimestamp systemTimestamp;
            systemTimestamp.set_nanos(i);
            EXPECT_TRUE(protobufWriter.writeNext(systemTimestamp));
        }
    }

    ProtobufReader protobufReader(&buf);
    for (size_t i = 0; i < numSamplesToWrite; i++) {
        core::SystemTimestamp systemTimestamp;
        EXPECT_TRUE(protobufReader.readNext(systemTimestamp));
        EXPECT_EQ(i, systemTimestamp.nanos());
    }
}
