#include "packages/feature_tracker/include/stereo_feature_matcher.h"
#include "packages/calibration/proto/camera_intrinsic_calibration.pb.h"
#include "packages/core/include/aligned_memory_allocator.h"
#include "packages/core/include/dynamic_image.h"
#include "packages/core/include/dynamic_storage.h"
#include "packages/core/include/unmanaged_storage.h"

#include "gtest/gtest.h"

using namespace core;

typedef DynamicStorage<uint8_scalar_t, AlignedMemoryAllocator> uint8_aligned_storage_t;
typedef UnmanagedStorage<uint8_pixel_t> uint8_unmanaged_storage_t;

TEST(StereoFeatureMatcher, matchFeatureStores) {
    static constexpr size_t rows = 320;
    static constexpr size_t cols = 240;
    static constexpr size_t stride = 325;

    calibration::CameraIntrinsicCalibration leftIntrinsics;
    calibration::CameraIntrinsicCalibration rightIntrinsics;
    leftIntrinsics.set_scaledfocallengthx(1);
    leftIntrinsics.set_scaledfocallengthy(1);
    leftIntrinsics.set_skew(0);
    leftIntrinsics.set_opticalcenterx(120);
    leftIntrinsics.set_opticalcentery(160);
    leftIntrinsics.set_resolutionx(240);
    leftIntrinsics.set_resolutiony(320);
    leftIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    leftIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    leftIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    leftIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    rightIntrinsics.set_scaledfocallengthx(1);
    rightIntrinsics.set_scaledfocallengthy(1);
    rightIntrinsics.set_skew(0);
    rightIntrinsics.set_opticalcenterx(120);
    rightIntrinsics.set_opticalcentery(160);
    rightIntrinsics.set_resolutionx(240);
    rightIntrinsics.set_resolutiony(320);
    rightIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    rightIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    rightIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    rightIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);

    core::DynamicImage<ImageType::uint8, uint8_aligned_storage_t> leftImage(rows, cols, stride);
    core::DynamicImage<ImageType::uint8, uint8_aligned_storage_t> rightImage(rows, cols, stride);

    memset(leftImage.view().data, 0, leftImage.rows() * leftImage.stride());
    memset(rightImage.view().data, 0, rightImage.rows() * rightImage.stride());

    leftImage.view().at(20, 20) = 10;
    leftImage.view().at(100, 100) = 100;

    rightImage.view().at(20, 20) = 10;
    rightImage.view().at(100, 100) = 100;

    std::vector<feature_detectors::FeaturePoint> leftPoints;
    feature_detectors::FeaturePoint leftPoint1;
    leftPoint1.set_x(20);
    leftPoint1.set_y(20);
    feature_detectors::FeaturePoint leftPoint2;
    leftPoint2.set_x(100);
    leftPoint2.set_y(100);
    leftPoints.push_back(leftPoint1);
    leftPoints.push_back(leftPoint2);

    std::vector<feature_detectors::FeaturePoint> rightPoints;
    feature_detectors::FeaturePoint rightPoint1;
    rightPoint1.set_x(20);
    rightPoint1.set_y(20);
    feature_detectors::FeaturePoint rightPoint2;
    rightPoint2.set_x(100);
    rightPoint2.set_y(100);
    rightPoints.push_back(rightPoint1);
    rightPoints.push_back(rightPoint2);

    auto leftImageView = leftImage.view();
    auto rightImageView = rightImage.view();

    Eigen::Matrix<float, 3, 3> rotation;
    float theta = 30.0 * M_PI / 180.0;
    rotation << cos(theta), -sin(theta), 0, sin(theta), cos(theta), 0, 0, 0, 1;
    Eigen::Matrix<float, 3, 1> translation(2, 3, 1);

    feature_tracker::StereoFeatureMatcher<11, float> matcher(16, -1, leftIntrinsics, rightIntrinsics, rotation, translation);
    const feature_tracker::NccFeatureStore<11> leftFeatureStore(leftImageView, leftPoints);
    const feature_tracker::NccFeatureStore<11> rightFeatureStore(rightImageView, rightPoints);

    const std::vector<std::pair<int, int> > matches = matcher.matchFeatureStores(leftFeatureStore, rightFeatureStore);

    EXPECT_EQ(2, matches.size());
    EXPECT_EQ(0, matches[0].first);
    EXPECT_EQ(0, matches[0].second);
    EXPECT_EQ(1, matches[1].first);
    EXPECT_EQ(1, matches[1].second);
}
