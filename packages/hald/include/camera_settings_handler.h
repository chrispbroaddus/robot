#pragma once

#include "packages/hal/proto/auto_exposure_roi_sample.pb.h"
#include "packages/hal/proto/camera_id.pb.h"
#include "packages/net/include/zmq_req_client.h"
#include "packages/teleop/proto/backend_message.pb.h"
#include "packages/teleop/proto/connection_options.pb.h"

namespace hald {

/// Class for changing camera settings during runtime
/// Currently only supports changing Auto-Exposure RoI
class CameraSettingsHandler {
public:
    CameraSettingsHandler(zmq::context_t& context, const teleop::ConnectionOptions& connectionOptions);

    bool setAutoExposureRoI(const teleop::ExposureCommand& cmd);
    bool resetAutoExposureRoI(const teleop::ResetExposureCommand& cmd);

private:
    typedef net::ZMQProtobufReqClient<hal::AutoExposureRoiCommand, hal::AutoExposureRoiResponse> ae_roi_client_t;

    std::map<hal::CameraId, std::unique_ptr<ae_roi_client_t> > m_AERoIClientMap;
};
}
