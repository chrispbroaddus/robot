#include "packages/unity_plugins/unity_telemetry_publisher/include/unity_telemetry_publisher.h"
#include "glog/logging.h"
#include "packages/unity_plugins/utils/include/coordinate_conversion.h"
#include "packages/unity_plugins/utils/include/zippy_image_interop.h"

using namespace unity_plugins;

std::unique_ptr<UnityTelemetryPublisher> unityPublisher;

extern "C" {
ZIPPY_INTERFACE_EXPORT void UnityTelemetryPublisher_initialize(
    const char* address, const int zmqLingerTimeInMilliSeconds, const int zmqHighWaterMarkValue) {
    unityPublisher.reset(new UnityTelemetryPublisher(std::string(address), zmqLingerTimeInMilliSeconds, zmqHighWaterMarkValue));
}

ZIPPY_INTERFACE_EXPORT void UnityTelemetryPublisher_stop() {
    if (unityPublisher) {
        unityPublisher.reset();
    }
}

///
/// \Brief Send the pose of the vehicle in rhs. +z : inverse gravity direction
/// \param tx
/// \param ty
/// \param tz
/// \param rx
/// \param ry
/// \param rz
/// \details It receives left-handed-coordinate pose from unity, then convert to right-handed.
///
ZIPPY_INTERFACE_EXPORT void UnityTelemetryPublisher_sendGroundTruthVehiclePose(float tx, float ty, float tz, float rx, float ry, float rz) {

    float rxp, ryp, rzp;
    float txp, typ, tzp;
    unity_plugins::convertUnityVehicleToZippyVehicleCoordinate(rxp, ryp, rzp, txp, typ, tzp, rx, ry, rz, tx, ty, tz);

    hal::Device* virtualVehicleDevice = new hal::Device();
    CHECK_NOTNULL(virtualVehicleDevice);
    virtualVehicleDevice->set_serialnumber(0x0000000000000000); // fixme : https://zippyai.atlassian.net/browse/AUTO-196
    virtualVehicleDevice->set_name("FrontLeftWheel");

    hal::Device* virtualUnityWorldDevice = new hal::Device();
    CHECK_NOTNULL(virtualUnityWorldDevice);
    virtualUnityWorldDevice->set_serialnumber(0x0000000000000000); // fixme : https://zippyai.atlassian.net/browse/AUTO-196
    virtualUnityWorldDevice->set_name("UnitySimulator");

    calibration::CoordinateFrame* vehicleCoordinate = new calibration::CoordinateFrame();
    CHECK_NOTNULL(vehicleCoordinate);
    vehicleCoordinate->set_allocated_device(virtualVehicleDevice);

    calibration::CoordinateFrame* unityCoordinate = new calibration::CoordinateFrame();
    CHECK_NOTNULL(vehicleCoordinate);
    unityCoordinate->set_allocated_device(virtualUnityWorldDevice);

    calibration::CoordinateTransformation transformation;
    transformation.set_allocated_sourcecoordinateframe(vehicleCoordinate);
    transformation.set_translationx(txp);
    transformation.set_translationy(typ);
    transformation.set_translationz(tzp);
    transformation.set_rodriguesrotationx(rxp);
    transformation.set_rodriguesrotationy(ryp);
    transformation.set_rodriguesrotationz(rzp);

    unityPublisher->sendGroundTruthVehiclePose(transformation);
}
}
