#include "Eigen/Eigen"
#include "glog/logging.h"
#include "google/protobuf/util/json_util.h"
#include "packages/calibration/proto/coordinate_transformation.pb.h"
#include "packages/core/include/chrono.h"
#include "packages/docking/proto/docking_station.pb.h"
#include "packages/perception/fiducials/proto/apriltag_config.pb.h"
#include "packages/perception/fiducials/proto/fiducial_configuration.pb.h"
#include "packages/unity_plugins/utils/include/coordinate_conversion.h"
#include "packages/unity_plugins/utils/include/zippy_interface.h"
#include "sophus/se3.hpp"
#include <fstream>

static docking::DockingStationList s_dockingStations;

extern "C" {
ZIPPY_INTERFACE_EXPORT void DockingStationJsonExporter_createDockingStation(int stationId) {

    bool exist = false;
    for (int i = 0; i < s_dockingStations.docking_stations_size(); i++) {
        if ((int)s_dockingStations.docking_stations(i).station_id() == stationId) {
            exist = true;
        }
    }
    if (!exist) {
        auto station = s_dockingStations.add_docking_stations();
        station->set_station_type(docking::StationType::VERTICAL_BOX);
        station->set_station_id(stationId);
    }
    LOG(INFO) << __FUNCTION__ << " ... created a docking_manager station.";
}

ZIPPY_INTERFACE_EXPORT void DockingStationJsonExporter_updateDockingStation(
    int stationId, int apriltagId, float rx, float ry, float rz, float x, float y, float z, int border, float sideLengthInMeters) {
    for (int i = 0; i < s_dockingStations.docking_stations_size(); i++) {
        if ((int)s_dockingStations.docking_stations(i).station_id() == stationId) {
            perception::AprilTagConfig* apriltagConfig = new perception::AprilTagConfig();
            CHECK_NOTNULL(apriltagConfig);

            apriltagConfig->set_border(border);
            apriltagConfig->set_apriltagfamily(perception::AprilTagFamily::AprilTag36h11);
            apriltagConfig->set_sidelengthinmeters(sideLengthInMeters);

            perception::FiducialConfiguration* fiducialConfiguration = new perception::FiducialConfiguration();
            CHECK_NOTNULL(fiducialConfiguration);

            fiducialConfiguration->set_allocated_apriltag_config(apriltagConfig);

            calibration::CoordinateFrame* targetCoordinateFrame = new calibration::CoordinateFrame();
            CHECK_NOTNULL(targetCoordinateFrame);

            hal::Device* targetCoordinateFrameDevice = new hal::Device();
            CHECK_NOTNULL(targetCoordinateFrameDevice);

            targetCoordinateFrameDevice->set_name("anchor_point");
            targetCoordinateFrame->set_allocated_device(targetCoordinateFrameDevice);

            calibration::CoordinateFrame* sourceCoordinateFrame = new calibration::CoordinateFrame();
            CHECK_NOTNULL(sourceCoordinateFrame);

            hal::Device* sourceCoordinateFrameDevice = new hal::Device();
            CHECK_NOTNULL(sourceCoordinateFrameDevice);

            sourceCoordinateFrameDevice->set_name("AprilTag36h11");
            sourceCoordinateFrameDevice->set_serialnumber(apriltagId);
            sourceCoordinateFrame->set_allocated_device(sourceCoordinateFrameDevice);

            calibration::CoordinateTransformation* transformation = new calibration::CoordinateTransformation();
            CHECK_NOTNULL(transformation);

            float rxp, ryp, rzp, txp, typ, tzp;
            unity_plugins::convertUnityAprilTagToDockingAprilTagCoordinate(rxp, ryp, rzp, txp, typ, tzp, rx, ry, rz, x, y, z);

            transformation->set_rodriguesrotationx(rxp);
            transformation->set_rodriguesrotationy(ryp);
            transformation->set_rodriguesrotationz(rzp);
            transformation->set_translationx(txp);
            transformation->set_translationy(typ);
            transformation->set_translationz(tzp);
            transformation->set_allocated_targetcoordinateframe(targetCoordinateFrame);
            transformation->set_allocated_sourcecoordinateframe(sourceCoordinateFrame);

            auto fiducial = s_dockingStations.mutable_docking_stations(i)->add_fiducials();
            fiducial->set_allocated_configuration(fiducialConfiguration);
            fiducial->set_allocated_transformation(transformation);

            LOG(INFO) << __FUNCTION__ << " ... Added a new fiducial on a docking_manager station.";
            LOG(INFO) << __FUNCTION__ << " ... ... apriltagId : " << apriltagId;
            LOG(INFO) << __FUNCTION__ << " ... ... pose (right-handed-coordinate)";
            LOG(INFO) << __FUNCTION__ << " ... ... rx, ry, rz : " << rxp << ", " << ryp << ", " << rzp;
            LOG(INFO) << __FUNCTION__ << " ... ... x, y, z : " << txp << ", " << typ << ", " << tzp;
        }
    }
}

void ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API DockingStationJsonExporter_export(const char* filename) {
    if (s_dockingStations.docking_stations_size() > 0) {
        LOG(INFO) << __FUNCTION__ << " ... # of stations : " << s_dockingStations.docking_stations_size();
        std::string msg;
        google::protobuf::util::MessageToJsonString(s_dockingStations, &msg);

        auto strFilename = std::string(filename);
        std::ofstream out(strFilename);
        out << msg;
        out.close();

        LOG(INFO) << __FUNCTION__ << " ... done.";
    }
}
}
