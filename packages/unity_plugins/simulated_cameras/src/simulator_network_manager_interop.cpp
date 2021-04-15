#include "packages/unity_plugins/simulated_cameras/include/simulator_network_manager.h"
#include "glog/logging.h"
#include "packages/hal/proto/camera_id.pb.h"
#include "packages/unity_plugins/simulated_cameras/include/simulator_network_interop.h"
#include "packages/unity_plugins/simulated_cameras/include/simulator_stats.h"
#include "packages/unity_plugins/utils/include/zippy_image_interop.h"
#include <thread>

using namespace unity_plugins;

namespace {
static std::unique_ptr<SimulatorNetworkManager> s_simulatorNetworkManager;
static std::mutex s_simulatorNetworkManagerMutex; // required as send image is called on a separate thread. so shutdown and send image may
// be called at the same time
}

///
/// @brief Callback to process a downloaded image. This is hooked up to the Texture Downloader
///
static void ZIPPY_INTERFACE_API SimulatedCameras_SendImage(CameraImage cameraImage) {

    // don't use guard, as this function gets called by multiple threads (1 per active camera output)
    // instead lock for the minimal amount of time
    s_simulatorNetworkManagerMutex.lock();
    if (s_simulatorNetworkManager) {
        auto pubGroup = s_simulatorNetworkManager->cameraPublisherGroup();
        s_simulatorNetworkManagerMutex.unlock();

        pubGroup->sendImage(cameraImage);
    } else {
        s_simulatorNetworkManagerMutex.unlock();
    }
}

extern "C" {

ZIPPY_INTERFACE_EXPORT void SimulatorNetworkManager_Shutdown() {
    std::lock_guard<std::mutex> guard(s_simulatorNetworkManagerMutex);
    if (s_simulatorNetworkManager) {
        auto ptr = s_simulatorNetworkManager.release();
        delete ptr;
    }
}

ZIPPY_INTERFACE_EXPORT void SimulatorNetworkManager_Initialize(const SimulatorNetworkManagerSettings settings) {
    SimulatorNetworkManager_Shutdown();
    s_simulatorNetworkManager.reset(new SimulatorNetworkManager(settings));
}

ZIPPY_INTERFACE_EXPORT void SimulatorNetworkManager_Update() {
    if (s_simulatorNetworkManager) {
        s_simulatorNetworkManager->update();
    }
}

///
/// @brief Get the function pointer to the save image function
///
ProcessImageFuncPtr ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API SimulatorNetworkManager_GetSendImageFuncPtr() {
    return SimulatedCameras_SendImage;
}

ZIPPY_INTERFACE_EXPORT void SimulatorNetworkManager_SendStats(const SimulatorStats stats) {
    if (s_simulatorNetworkManager) {
        s_simulatorNetworkManager->statsPublisher().sendStats(stats);
    }
}
}
