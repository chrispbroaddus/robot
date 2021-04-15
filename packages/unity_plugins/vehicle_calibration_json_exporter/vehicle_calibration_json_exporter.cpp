#include "glog/logging.h"
#include "google/protobuf/util/json_util.h"
#include "packages/calibration/proto/coordinate_transformation.pb.h"
#include "packages/core/include/chrono.h"
#include "packages/docking/proto/vehicle_calibration.pb.h"
#include "packages/math/geometry/quaternion.h"
#include "packages/unity_plugins/utils/include/coordinate_conversion.h"
#include "packages/unity_plugins/utils/include/zippy_interface.h"
#include <fstream>

extern "C" {

void ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API VehicleCalibrationJsonExporter_export(const char* filename, float fWrtCTx, float fWrtCTy,
    float fWrtCTz, float fWrtCQx, float fWrtCQy, float fWrtCQz, float fWrtCQw, float jpWrtFTx, float jpWrtFTy, float jpWrtFTz,
    float jpWrtFQx, float jpWrtFQy, float jpWrtFQz, float jpWrtFQw, float distBetweenWheel) {

    docking::VehicleCalibration calibration;

    calibration::CoordinateTransformation* se3fwrtc = new calibration::CoordinateTransformation();
    auto r = geometry::convertQuaternionToRodrigues(fWrtCQw, fWrtCQx, fWrtCQy, fWrtCQz);

    float rxZippy, ryZippy, rzZippy, txZippy, tyZippy, tzZippy;
    unity_plugins::convertUnityVehicleToZippyVehicleCoordinate(
        rxZippy, ryZippy, rzZippy, txZippy, tyZippy, tzZippy, r(0, 0), r(1, 0), r(2, 0), fWrtCTx, fWrtCTy, fWrtCTz);

    se3fwrtc->set_rodriguesrotationx(rxZippy);
    se3fwrtc->set_rodriguesrotationy(ryZippy);
    se3fwrtc->set_rodriguesrotationz(rzZippy);
    se3fwrtc->set_translationx(txZippy);
    se3fwrtc->set_translationy(tyZippy);
    se3fwrtc->set_translationz(tzZippy);

    calibration.set_allocated_se3fwrtc(se3fwrtc);

    calibration::CoordinateTransformation* se3JpwrtF = new calibration::CoordinateTransformation();
    auto rp = geometry::convertQuaternionToRodrigues(jpWrtFQw, jpWrtFQx, jpWrtFQy, jpWrtFQz);

    unity_plugins::convertUnityVehicleToZippyVehicleCoordinate(
        rxZippy, ryZippy, rzZippy, txZippy, tyZippy, tzZippy, rp(0, 0), rp(1, 0), rp(2, 0), jpWrtFTx, jpWrtFTy, jpWrtFTz);

    se3JpwrtF->set_rodriguesrotationx(rxZippy);
    se3JpwrtF->set_rodriguesrotationy(ryZippy);
    se3JpwrtF->set_rodriguesrotationz(rzZippy);
    se3JpwrtF->set_translationx(txZippy);
    se3JpwrtF->set_translationy(tyZippy);
    se3JpwrtF->set_translationz(tzZippy);

    calibration.set_allocated_se3jpwrtf(se3JpwrtF);

    calibration.set_distbetweenwheel(distBetweenWheel);

    std::string msg;
    google::protobuf::util::MessageToJsonString(calibration, &msg);

    auto strFilename = std::string(filename);
    std::ofstream out(strFilename);
    out << msg;
    out.close();
}
}
