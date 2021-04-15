#include "glog/logging.h"
#include "packages/core/include/chrono.h"

namespace {
// We need to keep a copy of the app name otherwise the passed in name will go out of scope and glog is too
// stupid to be memory safe and make its own copy
static std::string g_app_name;

// glog will seg fault if it is initialized more than once. Even though it can check its init status it prefers to kill everything
// and we cannot check its init status, so we have to maintain our own record of its status.
static bool g_glog_initialized = false;
}
extern "C" {

void Simulator_InitGLOG(const char* appName) {
    if (!g_glog_initialized) {
        g_app_name = std::string(appName);
        google::InitGoogleLogging(g_app_name.c_str());
        g_glog_initialized = true;
    }
}
}
