#include "gtest/gtest.h"

#include <chrono>

#include "packages/perception/perception.h"
#include "packages/perception/proto/perception_options.pb.h"
#include "packages/perception/terrain.h"
#include "packages/perception/utils.h"

#include "packages/calibration/proto/pinhole_camera_distortion_model.pb.h"

TEST(PerceptionTest, UnprojectionCacheTest) {
    using perception::TerrainRepresentation;
    using perception::UnprojectionLookup;
    using perception::PerceptionOptions;
    using perception::Perception;
    using perception::PointCloudXYZ;

    constexpr int rows = 384;
    constexpr int columns = 512;
    using calibration::CameraIntrinsicCalibration;
    using calibration::PinholeCameraDistortionModel;
    CameraIntrinsicCalibration camera;
    camera.mutable_cameraundercalibration()->set_name(perception::kDefaultCamera);
    *camera.mutable_pinhole() = PinholeCameraDistortionModel();
    camera.set_scaledfocallengthx(1.0);
    camera.set_scaledfocallengthy(1.0);
    camera.set_opticalcenterx(columns / 2);
    camera.set_opticalcentery(rows / 2);

    PerceptionOptions options;
    std::unique_ptr<TerrainRepresentation> terrain;
    std::unique_ptr<Perception> perception_ptr(new Perception(options, std::move(terrain)));
    ASSERT_TRUE(perception_ptr.get() != nullptr);
    ASSERT_TRUE(perception_ptr->addCamera(columns, rows, camera));

    std::pair<CameraIntrinsicCalibration, Perception::CameraInterface> cameras;
    ASSERT_TRUE(perception_ptr->camera(perception::kDefaultCamera, cameras));
    UnprojectionLookup projector(cameras.second);

    const auto& lookup = projector.lookup();
    ASSERT_EQ(lookup.cols(), rows * columns);

    Eigen::MatrixXf image_data(rows, columns);
    for (int i = 0; i < image_data.size(); ++i) {
        static int range_counter = 1.0;
        *(image_data.data() + i) = range_counter++;
    }

    using hal::Image;
    hal::Image image;
    image.set_data(image_data.data(), image_data.size() * sizeof(float));
    image.set_stride(1);
    image.set_rows(rows);
    image.set_cols(columns);

    PointCloudXYZ cloud;
    cloud.xyz = Eigen::MatrixXf(3, rows * columns);
    constexpr int kNumFrames = 30;

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < kNumFrames; ++i) {
        projector.unproject(image, cloud);
    }
    auto diff = std::chrono::steady_clock::now() - start;
    auto delta = std::chrono::duration<double, std::milli>(diff).count();
    LOG(INFO) << (rows * columns) << " in " << (delta / kNumFrames) << " ms";
    constexpr float kRequiredFrequency = 30.0;
    EXPECT_GT(1 / ((delta / kNumFrames) / 1000.0), kRequiredFrequency);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
