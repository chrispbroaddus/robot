#include "packages/calibration/include/linear_camera_model.h"

#include "gtest/gtest.h"

TEST(LinearCameraModelTest, projectUnprojectTest) {

    Eigen::Matrix<double, 3, 3> K;
    Eigen::Matrix<double, 2, 1> pixel;
    Eigen::Matrix<double, 2, 1> pixelReproj;
    Eigen::Matrix<double, 3, 1> ray;

    K << 500, 0, 320, 0, 500, 120, 0, 0, 1;
    pixel << 300, 200;

    calibration::LinearCameraModel<double> cameraModel(K);
    ray = cameraModel.unproject(pixel);
    pixelReproj = cameraModel.project(ray);

    EXPECT_NEAR(pixel(0), pixelReproj(0), 1.0e-12);
    EXPECT_NEAR(pixel(1), pixelReproj(1), 1.0e-12);
}

TEST(LinearCameraModelTest, unprojectProjectTest) {

    Eigen::Matrix<double, 3, 3> K;
    Eigen::Matrix<double, 2, 1> pixel;
    Eigen::Matrix<double, 3, 1> rayReproj;
    Eigen::Matrix<double, 3, 1> ray;

    K << 500, 0, 320, 0, 500, 120, 0, 0, 1;
    ray << 0.1, -0.08, 1;

    calibration::LinearCameraModel<double> cameraModel(K);
    pixel = cameraModel.project(ray);
    rayReproj = cameraModel.unproject(pixel);

    EXPECT_NEAR(ray(0), rayReproj(0), 1.0e-12);
    EXPECT_NEAR(ray(1), rayReproj(1), 1.0e-12);
}
