#include "dc1394/dc1394.h"
#include "glog/logging.h"

#include <iostream>

/// Prints the GUIDs of all the cameras connected to the host PC
int main() {

    dc1394_t* bus;
    dc1394error_t err;
    dc1394camera_list_t* list;
    uint32_t cameraIdx = 0;

    /// Create a bus to connect to cameras
    bus = dc1394_new();
    if (!bus) {
        dc1394_log_error("Dc1394: Bus creation failed");
        throw std::runtime_error("Dc1394: Bus creation failed");
    }

    /// Get a list of all connected cameras
    err = dc1394_camera_enumerate(bus, &list);
    if (err != DC1394_SUCCESS || !list) {
        dc1394_log_error("Dc1394: Camera Enumeration failed");
        dc1394_free(bus);
        throw std::runtime_error("Dc1394: Camera Enumeration failed");
    }
    /// Check to see if there are any connected cameras
    if (list->num == 0) {
        dc1394_log_error("No cameras found");
        dc1394_camera_free_list(list);
        dc1394_free(bus);
        throw std::runtime_error("No cameras found");
    }

    /// Print the GUIDs of the connected cameras
    for (cameraIdx = 0; cameraIdx < list->num; cameraIdx++) {
        LOG(INFO) << "Camera Idx: " << cameraIdx << " GUID: " << list->ids[cameraIdx].guid;
    }

    dc1394_camera_free_list(list);
    dc1394_free(bus);

    return 0;
}
