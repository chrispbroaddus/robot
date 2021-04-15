#include "gtest/gtest.h"

#include <algorithm>

#include "Eigen/Eigen"

#include "packages/calibration/proto/pinhole_camera_distortion_model.pb.h"
#include "packages/core/proto/geometry.pb.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/perception/perception.h"
#include "packages/perception/proto/perception_options.pb.h"
#include "packages/perception/utils.h"
#include "packages/planning/definitions.h"

/*
 *Test ability to integrate range images
 */
TEST(perception, imageConversion) {
    using hal::Image;
    using perception::imageToPointCloud;
    core::PointCloud3d cloud;
    constexpr int rows = 1024;
    constexpr int columns = 768;
    constexpr int dimension = 3;
    std::vector<float> image_data(dimension * rows * columns, 0);
    std::generate(image_data.begin(), image_data.end(), []() {
        static int counter = 0;
        return static_cast<float>(counter++);
    });
    hal::Image image;
    image.set_data(image_data.data(), image_data.size() * sizeof(float));
    image.set_stride(dimension);
    image.set_rows(rows);
    image.set_cols(columns);
    ASSERT_TRUE(imageToPointCloud(image, cloud));
    ASSERT_TRUE(cloud.points_size() == (rows * columns));
    ASSERT_TRUE(cloud.points(0).x() == 0);
    ASSERT_TRUE(cloud.points(0).y() == 1);
    ASSERT_TRUE(cloud.points(0).z() == 2);
    ASSERT_TRUE(cloud.points(1).x() == 3);
}

/*
 *Test basic initalization of the base perception object
 */
TEST(perception, init) {
    using perception::Perception;
    using perception::PerceptionOptions;
    using perception::TerrainRepresentation;
    PerceptionOptions options;
    std::unique_ptr<TerrainRepresentation> terrain;
    std::unique_ptr<Perception> ptr(new Perception(options, std::move(terrain)));
    ASSERT_NE(ptr.get(), nullptr);
}

/*
 *Test integration of images into terrain representation
 */
namespace perception {
class DummyTerrainRepresentation : public TerrainRepresentation {
public:
    bool addPoints(const PointCloudXYZ& cloud) override { return (cloud.xyz.size() > 0); }
    bool intersect(__attribute__((unused)) const core::Ray3d& ray, __attribute__((unused)) core::Point3d& point) override { return false; }
    bool unobstructed(__attribute__((unused)) const std::vector<core::Point3d>& point) { return true; };
};
} // perception

TEST(perception, systemCalibration) {
    using perception::Perception;
    using perception::PerceptionOptions;
    using perception::TerrainRepresentation;
    using perception::DummyTerrainRepresentation;
    using perception::generateDummySystemCalibration;
    std::unique_ptr<TerrainRepresentation> terrain(new DummyTerrainRepresentation());
    PerceptionOptions options;
    std::unique_ptr<Perception> perception_ptr(new Perception(options, std::move(terrain)));
    ASSERT_NE(perception_ptr.get(), nullptr);
    calibration::SystemCalibration calibration = generateDummySystemCalibration();
    ASSERT_TRUE(perception_ptr->addDevices(calibration));
}

TEST(perception, imageUpdate) {
    using perception::Perception;
    using perception::PerceptionOptions;
    using perception::TerrainRepresentation;
    using perception::DummyTerrainRepresentation;
    using hal::CameraSample;
    using hal::Image;

    std::unique_ptr<TerrainRepresentation> terrain(new DummyTerrainRepresentation());
    PerceptionOptions options;
    std::unique_ptr<Perception> perception_ptr(new Perception(options, std::move(terrain)));
    ASSERT_NE(perception_ptr.get(), nullptr);

    CameraSample sample;
    sample.mutable_device()->set_name(perception::kDefaultCamera);
    auto image = sample.mutable_image();
    image->set_format(hal::Format::PB_RANGE);
    constexpr int rows = 1024;
    constexpr int columns = 768;
    constexpr int dimension = 3;
    std::vector<float> image_data(dimension * rows * columns, 0);
    std::generate(image_data.begin(), image_data.end(), []() {
        static int counter = 0;
        return static_cast<float>(counter++);
    });
    image->set_data(image_data.data(), image_data.size() * sizeof(float));
    image->set_stride(dimension);
    image->set_rows(rows);
    image->set_cols(columns);
    ASSERT_FALSE(perception_ptr->update(sample));

    using calibration::CameraIntrinsicCalibration;
    using calibration::PinholeCameraDistortionModel;
    CameraIntrinsicCalibration camera;
    camera.set_resolutionx(columns);
    camera.set_resolutiony(rows);
    *camera.mutable_pinhole() = PinholeCameraDistortionModel();
    camera.mutable_cameraundercalibration()->set_name(perception::kDefaultCamera);
    ASSERT_TRUE(perception_ptr->addCamera(camera));
    ASSERT_TRUE(perception_ptr->update(sample));
}

TEST(perception, teleopRelative) {
    using perception::Perception;
    using perception::PerceptionOptions;
    using perception::TerrainRepresentation;

    PerceptionOptions options;
    std::unique_ptr<TerrainRepresentation> terrain;
    std::unique_ptr<Perception> perception_ptr(new Perception(options, std::move(terrain)));
    ASSERT_NE(perception_ptr.get(), nullptr);

    using calibration::CameraIntrinsicCalibration;
    using calibration::PinholeCameraDistortionModel;
    const int width = 100;
    const int height = 100;

    CameraIntrinsicCalibration camera;
    camera.mutable_cameraundercalibration()->set_name(perception::kDefaultCamera);
    *camera.mutable_pinhole() = PinholeCameraDistortionModel();
    camera.set_resolutionx(width);
    camera.set_resolutiony(height);
    camera.set_scaledfocallengthx(1.0);
    camera.set_scaledfocallengthy(1.0);
    camera.set_opticalcenterx(50.0);
    camera.set_opticalcentery(50.0);

    teleop::PointAndGoCommand command;
    command.set_imagex(.5);
    command.set_imagey(.5);
    core::Ray3d ray;

    ASSERT_FALSE(perception_ptr->imageToRay(perception::kDefaultCamera, command, ray));
    ASSERT_TRUE(perception_ptr->addCamera(camera));
    ASSERT_FALSE(perception_ptr->addCamera(camera));
    ASSERT_FALSE(perception_ptr->imageToRay(perception::kDefaultCamera, command, ray));

    Sophus::SE3d extrinsics(Eigen::Quaterniond(Eigen::AngleAxisd{ 0, Eigen::Vector3d{ 0, 0, 0 } }), Eigen::Vector3d{ 0, 0, 0 });
    extrinsics.translation()[planning::kZAxis] = -1;
    ASSERT_TRUE(perception_ptr->addExtrinsics(perception::kDefaultCamera, extrinsics));
    ASSERT_FALSE(perception_ptr->addExtrinsics(perception::kDefaultCamera, extrinsics));
    ASSERT_TRUE(perception_ptr->imageToRay(perception::kDefaultCamera, command, ray));

    ASSERT_EQ(ray.direction().x(), 1);
    ASSERT_EQ(ray.direction().y(), 0);
    ASSERT_EQ(ray.direction().z(), 0);

    command.set_imagex(0);
    command.set_imagey(0);
    ASSERT_TRUE(perception_ptr->imageToRay(perception::kDefaultCamera, command, ray));
    ASSERT_EQ(ray.direction().x(), 1);
    ASSERT_EQ(ray.direction().y(), -50);
    ASSERT_EQ(ray.direction().z(), -50);

    command.set_imagex(1);
    command.set_imagey(1);
    ASSERT_TRUE(perception_ptr->imageToRay(perception::kDefaultCamera, command, ray));
    ASSERT_EQ(ray.direction().x(), 1);
    ASSERT_EQ(ray.direction().y(), 50);
    ASSERT_EQ(ray.direction().z(), 50);
}

TEST(perception, teleopAbsolute) {
    using perception::Perception;
    using perception::PerceptionOptions;
    using perception::FlatTerrain;
    using perception::TerrainRepresentation;

    PerceptionOptions options;
    std::unique_ptr<TerrainRepresentation> terrain(new FlatTerrain);
    std::unique_ptr<Perception> perception_ptr(new Perception(options, std::move(terrain)));
    ASSERT_NE(perception_ptr.get(), nullptr);

    using calibration::CameraIntrinsicCalibration;
    using calibration::PinholeCameraDistortionModel;
    CameraIntrinsicCalibration camera;
    camera.mutable_cameraundercalibration()->set_name(perception::kDefaultCamera);
    *camera.mutable_pinhole() = PinholeCameraDistortionModel();
    camera.set_scaledfocallengthx(1.0);
    camera.set_scaledfocallengthy(1.0);
    camera.set_opticalcenterx(50.0);
    camera.set_opticalcentery(50.0);
    constexpr int columns = 100;
    constexpr int rows = 100;
    camera.set_resolutionx(columns);
    camera.set_resolutiony(rows);
    ASSERT_TRUE(perception_ptr->addCamera(camera));

    Sophus::SE3d extrinsics;
    extrinsics.translation()[planning::kZAxis] = 1;
    ASSERT_TRUE(perception_ptr->addExtrinsics(perception::kDefaultCamera, extrinsics));

    teleop::PointAndGoCommand command;
    command.set_imagex(.5);
    command.set_imagey(.5);
    core::Ray3d ray;

    ASSERT_TRUE(perception_ptr->imageToRay(perception::kDefaultCamera, command, ray));
    ASSERT_EQ(ray.direction().x(), 1);
    ASSERT_EQ(ray.direction().y(), 0);
    ASSERT_EQ(ray.direction().z(), 0);

    core::Point3d intersection;
    ASSERT_FALSE(perception_ptr->imageToPoint(perception::kDefaultCamera, command, intersection));
    for (float y = 0; y <= 1.0f; y += .1f) {
        for (float x = 0; x <= 1.0f; x += .1f) {
            command.set_imagex(x);
            command.set_imagey(y);
            if (y > .5f) {
                ASSERT_TRUE(perception_ptr->imageToPoint(perception::kDefaultCamera, command, intersection));
                ASSERT_TRUE(intersection.x() > 0);
                if (x < 0.5f) {
                    ASSERT_TRUE(intersection.y() < 0);
                } else {
                    ASSERT_TRUE(intersection.y() >= 0);
                }
            } else {
                ASSERT_FALSE(perception_ptr->imageToPoint(perception::kDefaultCamera, command, intersection));
            }
        }
    }
}

TEST(perception, teleopMultipleCameras) {
    using perception::Perception;
    using perception::PerceptionOptions;
    using perception::TerrainRepresentation;

    PerceptionOptions options;
    std::unique_ptr<TerrainRepresentation> terrain;
    std::unique_ptr<Perception> perception_ptr(new Perception(options, std::move(terrain)));
    ASSERT_NE(perception_ptr.get(), nullptr);

    using calibration::CameraIntrinsicCalibration;
    using calibration::PinholeCameraDistortionModel;
    const int width = 100;
    const int height = 100;

    CameraIntrinsicCalibration camera;
    *camera.mutable_pinhole() = PinholeCameraDistortionModel();
    camera.set_resolutionx(width);
    camera.set_resolutiony(height);
    camera.set_scaledfocallengthx(1.0);
    camera.set_scaledfocallengthy(1.0);
    camera.set_opticalcenterx(50.0);
    camera.set_opticalcentery(50.0);

    teleop::PointAndGoCommand command;
    command.set_imagex(.5);
    command.set_imagey(.5);
    core::Ray3d ray;

    {
        // Right
        const std::string cameraName = "leftCamera";
        camera.mutable_cameraundercalibration()->set_name(cameraName);
        ASSERT_TRUE(perception_ptr->addCamera(camera));
        auto rotation = Eigen::Quaterniond(Eigen::AngleAxisd(-M_PI / 2.0, Eigen::Vector3d{ 0, 0, 1 }));
        Sophus::SE3d extrinsics(rotation, Eigen::Vector3d{ 0, 0, 1 });
        ASSERT_TRUE(perception_ptr->addExtrinsics(cameraName, extrinsics));
        ASSERT_TRUE(perception_ptr->imageToRay(cameraName, command, ray));
        ASSERT_NEAR(ray.direction().x(), 0, 1e-10);
        ASSERT_NEAR(ray.direction().y(), 1, 1e-10);
        ASSERT_NEAR(ray.direction().z(), 0, 1e-10);
    }
    {
        // Left
        const std::string cameraName = "rightCamera";
        camera.mutable_cameraundercalibration()->set_name(cameraName);
        ASSERT_TRUE(perception_ptr->addCamera(camera));
        auto rotation = Eigen::Quaterniond(Eigen::AngleAxisd(M_PI / 2.0, Eigen::Vector3d{ 0, 0, 1 }));
        Sophus::SE3d extrinsics(rotation, Eigen::Vector3d{ 0, 0, 1 });
        ASSERT_TRUE(perception_ptr->addExtrinsics(cameraName, extrinsics));
        ASSERT_TRUE(perception_ptr->imageToRay(cameraName, command, ray));
        ASSERT_NEAR(ray.direction().x(), 0, 1e-10);
        ASSERT_NEAR(ray.direction().y(), -1, 1e-10);
        ASSERT_NEAR(ray.direction().z(), 0, 1e-10);
    }
    {
        // Backwards
        const std::string cameraName = "rearCamera";
        camera.mutable_cameraundercalibration()->set_name(cameraName);
        ASSERT_TRUE(perception_ptr->addCamera(camera));
        auto rotation = Eigen::Quaterniond(Eigen::AngleAxisd(M_PI, Eigen::Vector3d{ 0, 0, 1 }));
        Sophus::SE3d extrinsics(rotation, Eigen::Vector3d{ 0, 0, 1 });
        ASSERT_TRUE(perception_ptr->addExtrinsics(cameraName, extrinsics));
        ASSERT_TRUE(perception_ptr->imageToRay(cameraName, command, ray));
        ASSERT_NEAR(ray.direction().x(), -1, 1e-10);
        ASSERT_NEAR(ray.direction().y(), 0, 1e-10);
        ASSERT_NEAR(ray.direction().z(), 0, 1e-10);
    }
    {
        // Forwards
        const std::string cameraName = "forwardsCamera";
        camera.mutable_cameraundercalibration()->set_name(cameraName);
        ASSERT_TRUE(perception_ptr->addCamera(camera));
        auto rotation = Eigen::Quaterniond(Eigen::AngleAxisd(0, Eigen::Vector3d{ 0, 0, 1 }));
        Sophus::SE3d extrinsics(rotation, Eigen::Vector3d{ 0, 0, 1 });
        ASSERT_TRUE(perception_ptr->addExtrinsics(cameraName, extrinsics));
        ASSERT_TRUE(perception_ptr->imageToRay(cameraName, command, ray));
        ASSERT_NEAR(ray.direction().x(), 1, 1e-10);
        ASSERT_NEAR(ray.direction().y(), 0, 1e-10);
        ASSERT_NEAR(ray.direction().z(), 0, 1e-10);
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
