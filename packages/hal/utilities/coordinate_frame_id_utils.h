#pragma once

#include "packages/hal/proto/camera_id.pb.h"
#include "packages/hal/proto/coordinate_frame_id.pb.h"
#include <stdexcept>

namespace hal {

///
/// \param Retrieve the CoordinateFrameId from the name of the device
/// \return
inline CoordinateFrameId getCoordinateFrameIdFromDeviceName(const std::string& name) {

    // The name should be one of these
    CoordinateFrameId coordFrameId;
    if (CoordinateFrameId_Parse(name, &coordFrameId)) {
        return coordFrameId;
    }

    // Due to historical reasons, it may not have CoordFrame in front, so append and test
    std::string appendedName = "CoordFrame" + name;
    if (CoordinateFrameId_Parse(appendedName, &coordFrameId)) {
        return coordFrameId;
    }

    throw std::runtime_error("The coordinate frame (" + name + ") is not a valid registered coordinate name.");
}
}
