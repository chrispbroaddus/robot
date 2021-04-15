#include "gtest/gtest.h"

#include "packages/perception/perception.h"
#include "packages/perception/proto/perception_options.pb.h"
#include "packages/perception/terrain.h"
#include "packages/perception/utils.h"

#include "packages/calibration/proto/camera_intrinsic_calibration.pb.h"
#include "packages/calibration/proto/pinhole_camera_distortion_model.pb.h"

TEST(utils, initialise) {
    using calibu::Rig;
    std::unique_ptr<Rig<double> > _rig(new Rig<double>);
    ASSERT_TRUE(_rig.get() != nullptr);
}

class MockCameraInterface : public calibu::CameraInterface<double> {
    typedef Eigen::Matrix<double, 2, 1> Vec2t;
    typedef Eigen::Matrix<double, 3, 1> Vec3t;
    typedef Eigen::Matrix<double, 2, 3> d_Vec3t;

public:
    MockCameraInterface() {}

    void Scale(const double&) override {}

    void PrintInfo() const override {}

    Eigen::Matrix<double, 3, 3> K() const override {
        Eigen::Matrix<double, 3, 3> ret;
        return ret;
    }

    Vec3t Unproject(__attribute__((unused)) const Vec2t& pix) const override { return Vec3t(); }

    Vec2t Project(__attribute__((unused)) const Vec3t& ray) const override { return Vec2t(); }

    d_Vec3t dProject_dray(__attribute__((unused)) const Vec3t& ray) const override { return d_Vec3t(); }

    Eigen::Matrix<double, 2, Eigen::Dynamic> dProject_dparams(__attribute__((unused)) const Vec3t& ray) const override {
        return Eigen::Matrix<double, 2, 2>();
    }

    Eigen::Matrix<double, 3, Eigen::Dynamic> dUnproject_dparams(__attribute__((unused)) const Vec2t& pix) const override {
        return Eigen::Matrix<double, 3, 2>();
    }
};

TEST(utils, xmlIO) {
    using calibu::Rig;
    using calibu::CameraInterface;
    using calibu::WriteXmlRig;
    using calibu::ReadXmlRig;
    char filename[] = "/tmp/perception_utils_test_calibu_io_XXXXXX";
    int fd = mkstemp(filename);
    ASSERT_TRUE(fd > 0);
    {
        std::shared_ptr<Rig<double> > _rig(new Rig<double>);
        ASSERT_TRUE(_rig.get() != nullptr);
        std::shared_ptr<CameraInterface<double> > _camera(new MockCameraInterface);
        Eigen::VectorXd params(5);
        params[0] = params[1] = params[2] = params[3] = params[4] = 11.1;
        _camera->SetParams(params);
        _camera->SetImageDimensions(100, 200);
        _rig->AddCamera(_camera);
        WriteXmlRig(filename, _rig);
        ASSERT_THROW(ReadXmlRig(filename), std::runtime_error);
    }
    {
        std::shared_ptr<Rig<double> > _rig(new Rig<double>);
        ASSERT_TRUE(_rig.get() != nullptr);
        std::shared_ptr<CameraInterface<double> > _camera(new MockCameraInterface);
        Eigen::VectorXd params(5);
        params[0] = params[1] = params[2] = params[3] = params[4] = 22.2;
        _camera->SetParams(params);
        _camera->SetImageDimensions(100, 200);
        _camera->SetType("calibu_fu_fv_u0_v0");

        Eigen::Matrix3d rdf;
        rdf.col(0) = Eigen::Vector3d({ 0, 0, 1 });
        rdf.col(1) = Eigen::Vector3d({ 0, 1, 0 });
        rdf.col(2) = Eigen::Vector3d({ 1, 0, 0 });
        _camera->SetRDF(rdf);

        _rig->AddCamera(_camera);
        WriteXmlRig(filename, _rig);
        // TODO: Currently throws
        // ASSERT_NO_THROW(ReadXmlRig(filename));
    }
    CHECK(remove(filename) == 0);
}

/*
 *Test ability to unproject range images
 */
TEST(perception, unprojectionCache) {
    using perception::TerrainRepresentation;
    using perception::UnprojectionLookup;
    using perception::PerceptionOptions;
    using perception::Perception;
    using perception::PointCloudXYZ;

    constexpr int rows = 384;
    constexpr int columns = 512;
    constexpr int points = rows * columns;

    using calibration::CameraIntrinsicCalibration;
    using calibration::PinholeCameraDistortionModel;
    CameraIntrinsicCalibration camera;
    camera.mutable_cameraundercalibration()->set_name(perception::kDefaultCamera);
    *camera.mutable_pinhole() = PinholeCameraDistortionModel();
    camera.set_scaledfocallengthx(1.0);
    camera.set_scaledfocallengthy(1.0);
    camera.set_opticalcenterx(columns / 2);
    camera.set_opticalcentery(rows / 2);
    camera.set_resolutiony(rows);
    camera.set_resolutionx(columns);

    PerceptionOptions options;
    std::unique_ptr<TerrainRepresentation> terrain;
    std::unique_ptr<Perception> perception_ptr(new Perception(options, std::move(terrain)));
    ASSERT_TRUE(perception_ptr.get() != nullptr);
    ASSERT_TRUE(perception_ptr->addCamera(camera));

    std::pair<CameraIntrinsicCalibration, Perception::CameraInterface> cameras;
    ASSERT_TRUE(perception_ptr->camera(perception::kDefaultCamera, cameras));
    UnprojectionLookup projector(cameras.second);

    const auto& lookup = projector.lookup();
    ASSERT_EQ(lookup.cols(), rows * columns);

    Eigen::MatrixXf image_data(rows, columns);
    for (int i = 0; i < image_data.size(); ++i) {
        static int range_counter = 1.0;
        *(image_data.data() + i) = range_counter;
    }

    using hal::Image;
    hal::Image image;
    image.set_data(image_data.data(), image_data.size() * sizeof(float));
    image.set_stride(1);
    image.set_rows(rows);
    image.set_cols(columns);

    PointCloudXYZ cloud;
    cloud.xyz = Eigen::MatrixXf(3, rows * columns);
    projector.unproject(image, cloud);
    ASSERT_EQ(points, cloud.xyz.cols());
    for (int i = 0; i < cloud.xyz.cols(); ++i) {
        auto point = cloud.xyz.block<3, 1>(0, i);
        ASSERT_GT(point[2], 0.0);
    }
}

TEST(perception, pointCloudTransform) {
    using perception::PointCloudXYZ;

    constexpr int rows = 384;
    constexpr int columns = 512;
    constexpr int points = rows * columns;

    PointCloudXYZ cloud;
    cloud.xyz = Eigen::MatrixXf(3, points);
    for (int i = 0; i < points; ++i) {
        cloud.xyz.block<3, 1>(0, i) = Eigen::Vector3f{ 0, 0, 1 };
    }
    Eigen::AngleAxisd rotation(M_PI / 2.0, Eigen::Vector3d{ 1, 0, 0 });
    Sophus::SE3d T(Eigen::Quaterniond(rotation), Eigen::Vector3d{ 0, 0, 0 });
    transform(cloud, T);
    for (int i = 0; i < points; ++i) {
        auto y = cloud.xyz.block<3, 1>(0, i)(1);
        ASSERT_FLOAT_EQ(y, 1.0);
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
