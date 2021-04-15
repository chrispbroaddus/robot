#include "packages/calibration/include/kb4_image_undistortion.h"
#include "packages/hal/proto/camera_sample.pb.h"

#include "gtest/gtest.h"

TEST(KB4ImageUndistortionTest, undistortionTest) {

    Eigen::Matrix<float, 3, 3> K;
    Eigen::Matrix<float, 4, 1> coef;
    K << 771.4914, 0, 1244.208, 0, 770.1611, 1077.472, 0, 0, 1;
    coef << 0.01117972, 0.04504434, -0.05763411, 0.02156141;

    Eigen::Matrix<float, 3, 3> newK;
    float newFocalLength = 0.25f * K(0, 0);
    uint32_t outputRows = 512;
    uint32_t outputCols = 612;
    newK << newFocalLength, 0, outputCols / 2, 0, newFocalLength, outputRows / 2, 0, 0, 1;

    hal::Image distortedImage;
    hal::Image undistortedImage;
    std::vector<unsigned char> imageData;
    imageData.resize(2448 * 2048);

    calibration::KannalaBrandtRadialDistortionModel4<float> kb4Model(K, coef, 10, 0);
    calibration::LinearCameraModel<float> linearModel(newK);

    Eigen::Matrix<float, 2, 1> outputPixelLoc(200, 400);
    Eigen::Matrix<float, 3, 1> ray = linearModel.unproject(outputPixelLoc);
    Eigen::Matrix<float, 2, 1> inputPixelLoc = kb4Model.project(ray);

    imageData[(int)std::round(inputPixelLoc(1)) * 2448 + (int)std::round(inputPixelLoc(0))] = 30;

    distortedImage.set_data(imageData.data(), imageData.size());
    distortedImage.set_rows(2048);
    distortedImage.set_cols(2448);
    distortedImage.set_stride(2448);
    distortedImage.set_type(hal::PB_UNSIGNED_BYTE);
    distortedImage.set_format(hal::PB_LUMINANCE);

    calibration::Kb4ImageUndistortion<float> kb4ImageUndistortion(K, coef, outputRows, outputCols, newK);
    kb4ImageUndistortion.undistortImage(distortedImage, undistortedImage);

    unsigned char* undistortedImagePtr = (unsigned char*)undistortedImage.mutable_data()->data();
    EXPECT_EQ(30, undistortedImagePtr[400 * outputCols + 200]);
}
