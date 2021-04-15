#include "packages/perception/perception.h"

#include "glog/logging.h"
#include "wykobi/wykobi.hpp"

#include "packages/planning/definitions.h"
#include "packages/planning/utils.h"

namespace perception {

using calibration::CameraIntrinsicCalibration;

bool Perception::update(const hal::CameraSample& sample, std::string* deviceToUse) {
    switch (sample.image().format()) {
    case hal::Format::PB_RANGE:
        return updateRangeImage(sample, deviceToUse);
        break;
    case hal::Format::PB_POINTCLOUD:
        return updateCloudImage(sample, deviceToUse);
        break;
    default:
        LOG(FATAL) << "Unknown image type!";
    }
}

bool Perception::updateRangeImage(const hal::CameraSample& sample, std::string* deviceToUse) {
    const int rows = sample.image().rows();
    const int cols = sample.image().cols();
    CHECK(rows > 0);
    CHECK(cols > 0);

    std::string device_name;
    if (deviceToUse != nullptr) {
        device_name = *deviceToUse;
    } else {
        device_name = sample.device().name();
    }
    CHECK(!device_name.empty());
    if (m_projector.find(device_name) == m_projector.end()) {
        LOG(ERROR) << "Failed to find projector for: " << device_name;
        for (const auto& projector : m_projector) {
            LOG(INFO) << projector.first << std::endl;
        }
        return false;
    }
    auto& projector = m_projector.at(device_name);
    PointCloudXYZ cloud;
    cloud.xyz = Eigen::MatrixXf(3, rows * cols);
    projector.unproject(sample.image(), cloud);
    CHECK_NOTNULL(m_terrain);
    m_terrain->addPoints(cloud);
    return true;
}

bool Perception::updateCloudImage(const hal::CameraSample& sample, __attribute__((unused)) std::string* device_to_use) {
    const int rows = sample.image().rows();
    const int cols = sample.image().cols();
    CHECK(rows > 0);
    CHECK(cols > 0);
    PointCloudXYZ cloud;
    imageToPointCloud(sample.image(), cloud);
    perception::checkCloud(cloud, rows * cols);
    CHECK_NOTNULL(m_terrain);
    return m_terrain->addPoints(cloud);
}

bool Perception::imageToPoint(const std::string& camera, const teleop::PointAndGoCommand& command, core::Point3d& target) {
    CHECK_NOTNULL(m_terrain);
    core::Ray3d ray;
    CHECK(imageToRay(camera, command, ray));
    return m_terrain->intersect(ray, target);
}

bool Perception::camera(const std::string& name, std::pair<CameraIntrinsicCalibration, Perception::CameraInterface>& cameras) {
    if (m_cameras.find(name) == m_cameras.end()) {
        LOG(ERROR) << "Failed to find: " << name;
        return false;
    }
    cameras = m_cameras.at(name);
    return true;
}

bool Perception::extrinsics(const std::string& name, Sophus::SE3d& extrinsics) const {
    if (m_extrinsics.find(name) == m_extrinsics.end()) {
        LOG(ERROR) << "Failed to find: " << name;
        return false;
    }
    extrinsics = m_extrinsics.at(name);
    return true;
}

bool Perception::projector(const std::string& name, UnprojectionLookup*& projection) const {
    if (m_projector.find(name) == m_projector.end()) {
        projection = nullptr;
        LOG(ERROR) << "Failed to find: " << name;
        return false;
    }
    projection = &m_projector.at(name);
    return true;
}

bool Perception::imageToRay(const std::string& name, const teleop::PointAndGoCommand& command, core::Ray3d& target) {
    CHECK(command.imagex() >= 0 && command.imagex() <= 1);
    CHECK(command.imagey() >= 0 && command.imagey() <= 1);
    std::pair<CameraIntrinsicCalibration, CameraInterface> cameras;
    if (!this->camera(name, cameras)) {
        LOG(ERROR) << "Failed to retrieve: (" << command.camera() << ")";
        return false;
    }
    Sophus::SE3d robotToCamera;
    if (!this->extrinsics(name, robotToCamera)) {
        LOG(ERROR) << "Failed to index extrinsics for: (" << name << ")";
        return false;
    }
    const auto& calibu_camera = cameras.second;
    CHECK(calibu_camera->Width() > 0) << "This width is invalid!";
    CHECK(calibu_camera->Height() > 0) << "This height is invalid";
    Eigen::Vector2d query({ command.imagex() * calibu_camera->Width(), command.imagey() * calibu_camera->Height() });

    // Camera-centric co-ordinate system
    auto cvRay = calibu_camera->Unproject(query);

    // Robot-centric co-ordinate system
    Eigen::Vector3d zippyRay{ 0, 0, 0 };
    cvToZippy(cvRay, zippyRay);
    Eigen::Vector3d orientedRay = robotToCamera.so3().matrix() * zippyRay;

    core::Vector3d ray;
    ray.set_x(orientedRay[0]);
    ray.set_y(orientedRay[1]);
    ray.set_z(orientedRay[2]);
    *target.mutable_direction() = ray;

    core::Point3d position;
    auto translation = robotToCamera.translation();
    position.set_x(translation[planning::kXAxis]);
    position.set_y(translation[planning::kYAxis]);
    position.set_z(translation[planning::kZAxis]);
    *target.mutable_origin() = position;
    return true;
}

bool Perception::addCamera(const calibration::CameraIntrinsicCalibration& camera, bool force_linear) {
    auto name = camera.cameraundercalibration().name();
    CHECK(!name.empty()) << "Invalid camera name!";
    CHECK(camera.resolutionx() > 0) << camera.DebugString();
    CHECK(camera.resolutiony() > 0) << camera.DebugString();
    if (m_cameras.find(name) != m_cameras.end()) {
        return false;
    }

    CameraInterface calibu_camera;
    if (camera.has_pinhole() || force_linear) {
        Eigen::Vector4d params;
        params[0] = camera.scaledfocallengthx();
        params[1] = camera.scaledfocallengthy();
        params[2] = camera.opticalcenterx();
        params[3] = camera.opticalcentery();
        calibu_camera.reset(new calibu::LinearCamera<double>());
        CHECK_NOTNULL(calibu_camera.get());
        calibu_camera->SetParams(params);
        calibu_camera->SetImageDimensions(camera.resolutionx(), camera.resolutiony());
        m_cameras.insert(std::make_pair(name, std::make_pair(camera, calibu_camera)));
        UnprojectionLookup lookup(calibu_camera);
        m_projector.emplace(std::make_pair(name, std::move(lookup)));
        return true;
    } else if (camera.has_kannalabrandt()) {
        Eigen::VectorXd params(8);
        params[0] = camera.scaledfocallengthx();
        params[1] = camera.scaledfocallengthy();
        params[2] = camera.opticalcenterx();
        params[3] = camera.opticalcentery();
        constexpr int kExpectedNumberEntries = 4;
        CHECK(camera.kannalabrandt().radialdistortioncoefficientk_size() == kExpectedNumberEntries);
        params[4] = camera.kannalabrandt().radialdistortioncoefficientk(0);
        params[5] = camera.kannalabrandt().radialdistortioncoefficientk(1);
        params[6] = camera.kannalabrandt().radialdistortioncoefficientk(2);
        params[7] = camera.kannalabrandt().radialdistortioncoefficientk(3);

        calibu_camera.reset(new calibu::KannalaBrandtCamera<double>());
        calibu_camera->SetImageDimensions(camera.resolutionx(), camera.resolutiony());
        calibu_camera->SetParams(params);
        m_cameras.insert(std::make_pair(name, std::make_pair(camera, calibu_camera)));
        UnprojectionLookup lookup(calibu_camera);
        m_projector.emplace(std::make_pair(name, std::move(lookup)));
        return true;
    } else {
        LOG(ERROR) << "Unknown camera type!";
        return false;
    }
}

bool Perception::addExtrinsics(const std::string& name, const Sophus::SE3d& extrinsics) {
    if (m_extrinsics.find(name) != m_extrinsics.end()) {
        LOG(ERROR) << name << " already exists!";
        return false;
    }
    auto robotToCamera = extrinsics.inverse();
    auto z_offset = robotToCamera.translation()[planning::kZAxis];
    LOG_IF(WARNING, z_offset > 0) << "Invalid positive position for camera: " << name << " (" << z_offset << ")";
    m_extrinsics.insert(std::make_pair(name, robotToCamera));
    return true;
}

bool Perception::addDevices(const calibration::SystemCalibration calibration) {
    // This is done to allow for range-based iteration
    auto local_calibration = calibration;
    CHECK(local_calibration.cameraintrinsiccalibration_size() > 0);
    // Add in the intrinsics
    for (auto& camera_calibration : *local_calibration.mutable_cameraintrinsiccalibration()) {
        CHECK(addCamera(camera_calibration));
    }
    if (m_cameras.empty()) {
        LOG(ERROR) << "Failed to add any cameras!";
        return false;
    }
    // Add in the extrinsics
    for (const auto& camera : m_cameras) {
        bool extrinsicsFound = false;
        for (const auto& transformation : *local_calibration.mutable_devicetodevicecoordinatetransformation()) {
            if (transformation.targetcoordinateframe().device().name() == camera.first) {
                Sophus::SE3d extrinsics;
                planning::protoToPose(transformation, extrinsics);
                CHECK(addExtrinsics(camera.first, extrinsics));
                extrinsicsFound = true;
            }
        }
        if (!extrinsicsFound) {
            LOG(ERROR) << "Failed to find extrinsics for camera: " << camera.first;
            return false;
        }
    }
    return true;
}

} // perception
