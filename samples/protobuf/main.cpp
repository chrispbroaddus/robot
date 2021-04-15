#include "samples/protobuf/SampleMessage.pb.h"
#include <fstream>
#include <iostream>

#include "glog/logging.h"

int main(int argc, char* argv[]) {
    LOG(INFO) << "protobuf sample application";

    GOOGLE_PROTOBUF_VERIFY_VERSION;

    ProtocolBufferSample::Sample sample;

    sample.set_name("sample name");

    char file[] = "/tmp/fileXXXXXX";
    int descriptor = mkstemp(file);
    CHECK(descriptor != -1) << "Coult not create " << file;
    LOG(INFO) << "Writing to file: " << std::string(file);

    std::ofstream output(std::string(file), std::ios::out | std::ios::trunc | std::ios::binary);
    if (!sample.SerializeToOstream(&output)) {
        std::cerr << "Failed to write to file: " << std::string(file) << std::endl;
        return EXIT_FAILURE;
    }
    close(descriptor);

    google::protobuf::ShutdownProtobufLibrary();

    return EXIT_SUCCESS;
}
