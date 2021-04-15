
#include "glog/logging.h"
#include <iostream>

int main(int argc, char* argv[]) {
    google::InstallFailureSignalHandler();
    google::InitGoogleLogging(argv[0]);
    LOG(INFO) << "glog test";
    return 0;
}
