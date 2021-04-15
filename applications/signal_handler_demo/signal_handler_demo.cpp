#include "glog/logging.h"

int main(int argc, char* argv[]) {
    // Install failure signal handler so that glog will help dump
    // stacktrac when the binary crashes.
    google::InstallFailureSignalHandler();
    google::InitGoogleLogging(argv[0]);
    // Try to trigger SIGSEGV through null pointer dereference.
    int* a = nullptr;
    *a = 1;
    return 0;
}
