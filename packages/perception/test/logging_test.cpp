#include "gtest/gtest.h"

#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/perception/logging.h"

TEST(LoggingTest, Init) {
    using perception::LogReader;
    using perception::LogWriter;

    const char filename[] = "/tmp/test.dat";
    int written = 0;
    {
        LogWriter<hal::CameraSample> writer(filename);
        for (int i = 0; i < 1000; ++i) {
            hal::CameraSample sample;
            CHECK(writer.write(sample));
            ++written;
        }
    }
    int read = 0;
    {
        LogReader<hal::CameraSample> reader(filename);
        hal::CameraSample sample;
        while (reader.read(sample)) {
            ++read;
        }
    }
    ASSERT_EQ(read, written);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
