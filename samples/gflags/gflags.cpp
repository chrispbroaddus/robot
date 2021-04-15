
#include "gflags/gflags.h"
#include <iostream>

int main(int argc, char* argv[]) {
    gflags::SetUsageMessage("some usage message");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    std::cout << gflags::ProgramInvocationShortName() << std::endl;
    gflags::ShutDownCommandLineFlags();
    return 0;
}
