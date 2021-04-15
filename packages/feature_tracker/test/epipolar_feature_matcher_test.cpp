#include "packages/feature_tracker/include/epipolar_feature_matcher.h"
#include "packages/calibration/include/kannala_brandt_distortion_model.h"

#include "gtest/gtest.h"

TEST(EpipolarFeatureMatcherTest, canInstantiate) {

    calibration::CameraIntrinsicCalibration leftIntrinsics;
    calibration::CameraIntrinsicCalibration rightIntrinsics;
    leftIntrinsics.set_scaledfocallengthx(1);
    leftIntrinsics.set_scaledfocallengthy(1);
    leftIntrinsics.set_skew(0);
    leftIntrinsics.set_opticalcenterx(0);
    leftIntrinsics.set_opticalcentery(0);
    leftIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    leftIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    leftIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    leftIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    rightIntrinsics.set_scaledfocallengthx(1);
    rightIntrinsics.set_scaledfocallengthy(1);
    rightIntrinsics.set_skew(0);
    rightIntrinsics.set_opticalcenterx(0);
    rightIntrinsics.set_opticalcentery(0);
    rightIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    rightIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    rightIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    rightIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);

    Eigen::Matrix<float, 3, 3> rotation = Eigen::Matrix<float, 3, 3>::Identity();
    Eigen::Matrix<float, 3, 1> translation(0, 0, 0);
    float maxPointDistance = 10;
    float minMatchScore = 0.6;

    feature_tracker::EpipolarFeatureMatcher<11, float> featureMatcher(
        leftIntrinsics, rightIntrinsics, rotation, translation, maxPointDistance, minMatchScore, 480, 640);
}

TEST(EpipolarFeatureMatcherTest, singleFrameMatching) {

    calibration::CameraIntrinsicCalibration leftIntrinsics;
    calibration::CameraIntrinsicCalibration rightIntrinsics;
    leftIntrinsics.set_scaledfocallengthx(1);
    leftIntrinsics.set_scaledfocallengthy(1);
    leftIntrinsics.set_skew(0);
    leftIntrinsics.set_opticalcenterx(0);
    leftIntrinsics.set_opticalcentery(0);
    leftIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    leftIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    leftIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    leftIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    rightIntrinsics.set_scaledfocallengthx(1);
    rightIntrinsics.set_scaledfocallengthy(1);
    rightIntrinsics.set_skew(0);
    rightIntrinsics.set_opticalcenterx(0);
    rightIntrinsics.set_opticalcentery(0);
    rightIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    rightIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    rightIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    rightIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);

    constexpr size_t windowSize = 3;
    Eigen::Matrix<float, 3, 3> rotation = Eigen::Matrix<float, 3, 3>::Identity();
    Eigen::Matrix<float, 3, 1> translation(0, 0, 0);
    float maxPointDistance = 10;
    float minMatchScore = 0.6;
    constexpr uint32_t imageRows = 480;
    constexpr uint32_t imageCols = 640;
    std::vector<feature_tracker::NccFeature<windowSize> > nccFeatureVector;
    feature_tracker::NccFeature<windowSize> feature1;
    constexpr uint8_t imagePatch1[9] = { 1, 2, 3, 4, 2, 5, 6, 4, 6 };
    feature1.A = 33;
    feature1.B = 147;
    feature1.C = 0.065372045f;
    feature1.x = 2.1;
    feature1.y = 3.1;
    feature1.featurePointIndex = 3;
    memcpy(feature1.imagePatch, imagePatch1, 9);
    nccFeatureVector.push_back(feature1);
    feature_tracker::NccFeature<windowSize> feature2;
    constexpr uint8_t imagePatch2[9] = { 3, 4, 4, 4, 6, 2, 6, 2, 2 };
    feature2.A = 33;
    feature2.B = 141;
    feature2.C = 0.074535599f;
    feature2.x = 2.2;
    feature2.y = 3.2;
    feature2.featurePointIndex = 7;
    memcpy(feature2.imagePatch, imagePatch2, 9);
    nccFeatureVector.push_back(feature2);
    feature_tracker::NccFeatureStore<windowSize> featureStore(nccFeatureVector, imageRows, imageCols);

    feature_tracker::EpipolarFeatureMatcher<3, float> featureMatcher(
        leftIntrinsics, rightIntrinsics, rotation, translation, maxPointDistance, minMatchScore, 480, 640);

    featureMatcher.updateLeftFrame(&featureStore);
    std::vector<std::pair<int, int> > matchedIndices = featureMatcher.matchFeatures();
    EXPECT_EQ(0, matchedIndices.size());
}

TEST(EpipolarFeatureMatcherTest, trivialMatchingLinearModel) {

    calibration::CameraIntrinsicCalibration leftIntrinsics;
    calibration::CameraIntrinsicCalibration rightIntrinsics;
    leftIntrinsics.set_scaledfocallengthx(1);
    leftIntrinsics.set_scaledfocallengthy(1);
    leftIntrinsics.set_skew(0);
    leftIntrinsics.set_opticalcenterx(0);
    leftIntrinsics.set_opticalcentery(0);
    leftIntrinsics.set_resolutionx(2448);
    leftIntrinsics.set_resolutiony(2048);
    leftIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    leftIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    leftIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    leftIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    rightIntrinsics.set_scaledfocallengthx(1);
    rightIntrinsics.set_scaledfocallengthy(1);
    rightIntrinsics.set_skew(0);
    rightIntrinsics.set_opticalcenterx(0);
    rightIntrinsics.set_opticalcentery(0);
    rightIntrinsics.set_resolutionx(2448);
    rightIntrinsics.set_resolutiony(2048);
    rightIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    rightIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    rightIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);
    rightIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0);

    constexpr size_t windowSize = 3;
    Eigen::Matrix<float, 3, 3> rotation = Eigen::Matrix<float, 3, 3>::Identity();
    Eigen::Matrix<float, 3, 1> translation(2, 0, 0);
    float maxPointDistance = 15;
    float minMatchScore = 0.6;
    constexpr uint32_t imageRows = 2048;
    constexpr uint32_t imageCols = 2448;
    feature_tracker::EpipolarFeatureMatcher<3, float> featureMatcher(
        leftIntrinsics, rightIntrinsics, rotation, translation, maxPointDistance, minMatchScore, imageRows, imageCols);
    std::vector<feature_tracker::NccFeature<windowSize> > nccFeatureVector1;
    feature_tracker::NccFeature<windowSize> feature11;
    constexpr uint8_t imagePatch11[9] = { 1, 2, 3, 4, 2, 5, 6, 4, 6 };
    feature11.A = 33;
    feature11.B = 147;
    feature11.C = 0.065372045f;
    feature11.x = 30;
    feature11.y = 20;
    feature11.featurePointIndex = 3;
    memcpy(feature11.imagePatch, imagePatch11, 9);
    nccFeatureVector1.push_back(feature11);
    feature_tracker::NccFeature<windowSize> feature12;
    constexpr uint8_t imagePatch12[9] = { 3, 4, 4, 4, 6, 2, 6, 2, 2 };
    feature12.A = 33;
    feature12.B = 141;
    feature12.C = 0.074535599f;
    feature12.x = 20;
    feature12.y = 30;
    feature12.featurePointIndex = 7;
    memcpy(feature12.imagePatch, imagePatch12, 9);
    nccFeatureVector1.push_back(feature12);
    feature_tracker::NccFeatureStore<windowSize> featureStore1(nccFeatureVector1, imageRows, imageCols);

    std::vector<feature_tracker::NccFeature<windowSize> > nccFeatureVector2;
    feature_tracker::NccFeature<windowSize> feature21;
    constexpr uint8_t imagePatch21[9] = { 3, 4, 4, 4, 6, 2, 6, 2, 2 };
    feature21.A = 33;
    feature21.B = 141;
    feature21.C = 0.074535599f;
    feature21.x = 18;
    feature21.y = 30;
    feature21.featurePointIndex = 3;
    memcpy(feature21.imagePatch, imagePatch21, 9);
    nccFeatureVector2.push_back(feature21);
    feature_tracker::NccFeature<windowSize> feature22;
    constexpr uint8_t imagePatch22[9] = { 1, 2, 3, 4, 2, 5, 6, 4, 6 };
    feature22.A = 33;
    feature22.B = 147;
    feature22.C = 0.065372045f;
    feature22.x = 28;
    feature22.y = 20;
    feature22.featurePointIndex = 7;
    memcpy(feature22.imagePatch, imagePatch22, 9);
    nccFeatureVector2.push_back(feature22);
    feature_tracker::NccFeatureStore<windowSize> featureStore2(nccFeatureVector2, imageRows, imageCols);

    featureMatcher.updateLeftFrame(&featureStore1);
    featureMatcher.updateRightFrame(&featureStore2);
    std::vector<std::pair<int, int> > matchedIndices = featureMatcher.matchFeatures();
    EXPECT_EQ(2, matchedIndices.size());
    EXPECT_EQ(3, matchedIndices[0].first);
    EXPECT_EQ(7, matchedIndices[0].second);
    EXPECT_EQ(7, matchedIndices[1].first);
    EXPECT_EQ(3, matchedIndices[1].second);
}

TEST(EpipolarFeatureMatcherTest, matchingWithRotKB5Model) {

    calibration::CameraIntrinsicCalibration leftIntrinsics;
    calibration::CameraIntrinsicCalibration rightIntrinsics;
    leftIntrinsics.set_scaledfocallengthx(771.4914);
    leftIntrinsics.set_scaledfocallengthy(770.1611);
    leftIntrinsics.set_skew(0);
    leftIntrinsics.set_opticalcenterx(1244.208);
    leftIntrinsics.set_opticalcentery(1077.472);
    leftIntrinsics.set_resolutionx(2448);
    leftIntrinsics.set_resolutiony(2048);
    leftIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0.01117972);
    leftIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0.04504434);
    leftIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(-0.05763411);
    leftIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0.02156141);
    rightIntrinsics.set_scaledfocallengthx(761.5459);
    rightIntrinsics.set_scaledfocallengthy(760.4103);
    rightIntrinsics.set_skew(0);
    rightIntrinsics.set_opticalcenterx(1257.009);
    rightIntrinsics.set_opticalcentery(1050.432);
    rightIntrinsics.set_resolutionx(2448);
    rightIntrinsics.set_resolutiony(2048);
    rightIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0.08616139);
    rightIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(-0.08189862);
    rightIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(0.01098505);
    rightIntrinsics.mutable_kannalabrandt()->add_radialdistortioncoefficientk(-0.0004719968);

    Eigen::Matrix<float, 3, 3> leftK;
    Eigen::Matrix<float, 4, 1> leftDistortionCoef;
    Eigen::Matrix<float, 3, 3> rightK;
    Eigen::Matrix<float, 4, 1> rightDistortionCoef;

    feature_tracker::cameraIntrinsicCalibrationToEigen(leftIntrinsics, leftK, leftDistortionCoef);
    feature_tracker::cameraIntrinsicCalibrationToEigen(rightIntrinsics, rightK, rightDistortionCoef);

    calibration::KannalaBrandtRadialDistortionModel4<float> leftCameraModel(leftK, leftDistortionCoef, 10, 0);
    calibration::KannalaBrandtRadialDistortionModel4<float> rightCameraModel(rightK, rightDistortionCoef, 10, 0);
    constexpr size_t windowSize = 3;
    Eigen::Matrix<float, 3, 3> rotation;
    float theta = 30.0 * M_PI / 180.0;
    rotation << cos(theta), -sin(theta), 0, sin(theta), cos(theta), 0, 0, 0, 1;
    Eigen::Matrix<float, 3, 1> translation(2, 3, 1);

    Eigen::Matrix<float, 3, 1> point3dL1(30, 20, 10);
    Eigen::Matrix<float, 2, 1> point2dL1 = leftCameraModel.project(point3dL1.hnormalized().homogeneous());
    Eigen::Matrix<float, 2, 1> point2dR1
        = rightCameraModel.project((rotation.transpose() * point3dL1 - rotation.transpose() * translation).hnormalized().homogeneous());
    Eigen::Matrix<float, 3, 1> point3dL2(20, 40, 10);
    Eigen::Matrix<float, 2, 1> point2dL2 = leftCameraModel.project(point3dL2.hnormalized().homogeneous());
    Eigen::Matrix<float, 2, 1> point2dR2
        = rightCameraModel.project((rotation.transpose() * point3dL2 - rotation.transpose() * translation).hnormalized().homogeneous());
    float maxPointDistance = 15;
    float minMatchScore = 0.6;
    constexpr uint32_t imageRows = 2048;
    constexpr uint32_t imageCols = 2448;
    feature_tracker::EpipolarFeatureMatcher<3, float> featureMatcher(
        leftIntrinsics, rightIntrinsics, rotation, translation, maxPointDistance, minMatchScore, imageRows, imageCols);
    std::vector<feature_tracker::NccFeature<windowSize> > nccFeatureVector1;
    feature_tracker::NccFeature<windowSize> feature11;
    constexpr uint8_t imagePatch11[9] = { 1, 2, 3, 4, 2, 5, 6, 4, 6 };
    feature11.A = 33;
    feature11.B = 147;
    feature11.C = 0.065372045f;
    feature11.x = point2dL1.x();
    feature11.y = point2dL1.y();
    feature11.featurePointIndex = 3;
    memcpy(feature11.imagePatch, imagePatch11, 9);
    nccFeatureVector1.push_back(feature11);
    feature_tracker::NccFeature<windowSize> feature12;
    constexpr uint8_t imagePatch12[9] = { 3, 4, 4, 4, 6, 2, 6, 2, 2 };
    feature12.A = 33;
    feature12.B = 141;
    feature12.C = 0.074535599f;
    feature12.x = point2dL2.x();
    feature12.y = point2dL2.y();
    feature12.featurePointIndex = 7;
    memcpy(feature12.imagePatch, imagePatch12, 9);
    nccFeatureVector1.push_back(feature12);
    feature_tracker::NccFeatureStore<windowSize> featureStore1(nccFeatureVector1, imageRows, imageCols);

    std::vector<feature_tracker::NccFeature<windowSize> > nccFeatureVector2;
    feature_tracker::NccFeature<windowSize> feature21;
    constexpr uint8_t imagePatch21[9] = { 3, 4, 4, 4, 6, 2, 6, 2, 2 };
    feature21.A = 33;
    feature21.B = 141;
    feature21.C = 0.074535599f;
    feature21.x = point2dR2.x();
    feature21.y = point2dR2.y();
    feature21.featurePointIndex = 3;
    memcpy(feature21.imagePatch, imagePatch21, 9);
    nccFeatureVector2.push_back(feature21);
    feature_tracker::NccFeature<windowSize> feature22;
    constexpr uint8_t imagePatch22[9] = { 1, 2, 3, 4, 2, 5, 6, 4, 6 };
    feature22.A = 33;
    feature22.B = 147;
    feature22.C = 0.065372045f;
    feature22.x = point2dR1.x();
    feature22.y = point2dR1.y();
    feature22.featurePointIndex = 7;
    memcpy(feature22.imagePatch, imagePatch22, 9);
    nccFeatureVector2.push_back(feature22);
    feature_tracker::NccFeatureStore<windowSize> featureStore2(nccFeatureVector2, imageRows, imageCols);

    featureMatcher.updateLeftFrame(&featureStore1);
    featureMatcher.updateRightFrame(&featureStore2);
    std::vector<std::pair<int, int> > matchedIndices = featureMatcher.matchFeatures();
    EXPECT_EQ(2, matchedIndices.size());
    EXPECT_EQ(3, matchedIndices[0].first);
    EXPECT_EQ(7, matchedIndices[0].second);
    EXPECT_EQ(7, matchedIndices[1].first);
    EXPECT_EQ(3, matchedIndices[1].second);
}
