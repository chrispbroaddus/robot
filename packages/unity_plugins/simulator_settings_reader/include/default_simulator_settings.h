#pragma once

#include "packages/unity_plugins/simulator_settings_reader/include/simulator_settings_reader.h"
#include "packages/unity_simulator/proto/simulator_settings.pb.h"
#include <glog/logging.h>

namespace unity_plugins {
inline simulator_settings::SimulatorSettings defaultSimulatorSettings() {
    simulator_settings::SimulatorSettings settings;

    // vehicle section
    auto vehicle = new simulator_settings::SimulatedVehicleModel();
    CHECK_NOTNULL(vehicle);
    vehicle->set_model_id(simulator_settings::SimulatedVehicleModelId::BoxCar);

    auto vehicleCalibration = new simulator_settings::VehicleCalibration();
    CHECK_NOTNULL(vehicleCalibration);

    auto center = new core::Point2d();
    center->set_x(0);
    center->set_y(0);
    vehicleCalibration->set_allocated_center(center);

    auto frontLeft = new core::Point2d();
    frontLeft->set_x(-0.6290003);
    frontLeft->set_y(0.5000008);
    vehicleCalibration->set_allocated_frontleft(frontLeft);

    auto frontRight = new core::Point2d();
    frontRight->set_x(0.6289996);
    frontRight->set_y(0.4999985);
    vehicleCalibration->set_allocated_frontright(frontRight);

    auto middleLeft = new core::Point2d();
    middleLeft->set_x(-0.6290033);
    middleLeft->set_y(0);
    vehicleCalibration->set_allocated_middleleft(middleLeft);

    auto middleRight = new core::Point2d();
    middleRight->set_x(0.629001);
    middleRight->set_y(0);
    vehicleCalibration->set_allocated_middleright(middleRight);

    auto rearLeft = new core::Point2d();
    rearLeft->set_x(-0.6290016);
    rearLeft->set_y(-0.499999);
    vehicleCalibration->set_allocated_rearleft(rearLeft);

    auto rearRight = new core::Point2d();
    rearRight->set_x(0.6290025);
    rearRight->set_y(-0.4999998);
    vehicleCalibration->set_allocated_rearright(rearRight);

    vehicle->set_allocated_vehicle_calibration(vehicleCalibration);

    settings.set_allocated_vehicle(vehicle);

    // application section
    auto application = new simulator_settings::Application();
    CHECK_NOTNULL(application);
    application->set_targetframerate(30);
    application->set_targetfixeddeltatime(0.02f);
    settings.set_allocated_application(application);

    // cameras sectiojn
    auto cameras = new simulator_settings::Cameras();
    CHECK_NOTNULL(cameras);
    cameras->set_framedownloadbuffersize(2);
    cameras->set_camerastartdepth(-50.0f);
    cameras->set_maxtexturesize(8192);

    // camera section
    for (int ii = 0; ii < hal::CameraId_ARRAYSIZE; ++ii) {

        auto cameraId = (hal::CameraId)ii;

        auto camera = cameras->add_camera();
        CHECK_NOTNULL(camera);
        camera->set_cameraid((hal::CameraId)ii);
        camera->set_greyscale(ii < 4);
        camera->set_nearclippingplanedistancemeters(0.03f);
        camera->set_farclippingplanedistancemeters(500.0f);
        camera->set_distortedmeshrowfineness(0.1f);
        camera->set_distortedmeshcolfineness(0.1f);
        camera->set_calibratedplanescalefactor(1.0f);
        camera->set_maxdepthmapdistancemeters(50.0f);
        camera->set_imagescale(1.0f);

        if (cameraId == hal::CameraId::FrontFisheye) {
            camera->set_enabled(true);
            camera->set_depthenabled(true);
        } else {
            camera->set_enabled(false);
            camera->set_depthenabled(false);
        }

        switch (cameraId) {
        case hal::CameraId::FrontLeftStereo:
        case hal::CameraId::FrontRightStereo:
        case hal::CameraId::RearLeftStereo:
        case hal::CameraId::RearRightStereo:
            camera->set_imagezoom(1.0f);
            break;
        case hal::CameraId::FrontFisheye:
        case hal::CameraId::RearFisheye:
        case hal::CameraId::LeftFisheye:
        case hal::CameraId::RightFisheye:
            camera->set_imagezoom(10.0f);
            break;
        default:
            break;
        }

        camera->set_xyzenabled(false);
    }
    settings.set_allocated_cameras(cameras);

    // networking section
    auto networking = new simulator_settings::Networking();
    CHECK_NOTNULL(networking);
    networking->set_zmqsendreceivetimeoutms(1000);
    networking->set_zmqlingertimems(1000);
    networking->set_zmqhighwatermark(1);
    settings.set_allocated_networking(networking);

    // ground truth section
    auto groundTruth = new simulator_settings::GroundTruth();
    CHECK_NOTNULL(groundTruth);
    groundTruth->set_publishvehicleposition(false);
    groundTruth->set_publishfiducialposes(false);
    settings.set_allocated_groundtruth(groundTruth);

    // scene section
    auto scene = new simulator_settings::Scene();
    CHECK_NOTNULL(scene);
    scene->set_sceneurl("");
    settings.set_allocated_scene(scene);

    return settings;
}
}
