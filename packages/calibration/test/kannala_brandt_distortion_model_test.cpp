
#include "packages/calibration/include/kannala_brandt_distortion_model.h"
#include "packages/calibration/include/linear_camera_model.h"
#include "thirdparty/Calibu/include/calibu/cam/camera_models_kb4.h"
#include "gtest/gtest.h"

using namespace calibration;

TEST(KannalaBrandtRadialDistortionModel4, trivialProjectUnproject) {
    const Eigen::Matrix<double, 3, 1> ray = { .5, -.5, 1 };
    const Eigen::Matrix<double, 3, 3> K = Eigen::Matrix<double, 3, 3>::Identity();

    Eigen::Matrix<double, 4, 1> coef;
    coef(0) = 0;
    coef(1) = 0;
    coef(2) = 0;
    coef(3) = 0;

    KannalaBrandtRadialDistortionModel4<double> lensModel(K, coef, 10, 1e-10);

    const Eigen::Matrix<double, 2, 1> m = lensModel.project(ray);
    const Eigen::Matrix<double, 3, 1> rayP = lensModel.unproject(m);

    double rayArray[3] = { 0.5, -0.5, 1.0 };
    double params[8] = { 1, 1, 0, 0, 0, 0, 0, 0 };
    double pixArray[2] = { 0, 0 };
    double rayPArray[3] = { 0, 0, 0 };
    calibu::KannalaBrandtCamera<double>::Project(rayArray, params, pixArray);
    calibu::KannalaBrandtCamera<double>::Unproject(pixArray, params, rayPArray);

    EXPECT_DOUBLE_EQ(m(0), pixArray[0]);
    EXPECT_DOUBLE_EQ(m(1), pixArray[1]);
    EXPECT_DOUBLE_EQ(rayP(0), rayPArray[0] / rayPArray[2]);
    EXPECT_DOUBLE_EQ(rayP(1), rayPArray[1] / rayPArray[2]);
    EXPECT_DOUBLE_EQ(rayP(2), rayPArray[2] / rayPArray[2]);
    EXPECT_DOUBLE_EQ(ray(0), rayP(0));
    EXPECT_DOUBLE_EQ(ray(1), rayP(1));
    EXPECT_DOUBLE_EQ(ray(2), rayP(2));
}

TEST(KannalaBrandtRadialDistortionModel4, realCoefficientProjectUnproject) {

    double rayArray[3] = { 0.004, -0.002, 1.0 };
    double params[8] = { 1, 1, 0, 0, 0.01117972, 0.04504434, -0.05763411, 0.02156141 };
    const Eigen::Matrix<double, 3, 1> ray = { rayArray[0], rayArray[1], rayArray[2] };
    const Eigen::Matrix<double, 3, 3> K = Eigen::Matrix<double, 3, 3>::Identity();

    Eigen::Matrix<double, 4, 1> coef;

    // Real coefficients from a fish-eye calibration
    coef(0) = params[4];
    coef(1) = params[5];
    coef(2) = params[6];
    coef(3) = params[7];

    KannalaBrandtRadialDistortionModel4<double> lensModel(K, coef, 100, 0);
    const Eigen::Matrix<double, 2, 1> m = lensModel.project(ray);
    const Eigen::Matrix<double, 3, 1> rayP = lensModel.unproject(m);

    double pixArray[2] = { 0, 0 };
    double rayPArray[3] = { 0, 0, 0 };
    calibu::KannalaBrandtCamera<double>::Project(rayArray, params, pixArray);
    calibu::KannalaBrandtCamera<double>::Unproject(pixArray, params, rayPArray);

    EXPECT_NEAR(pixArray[0], m(0), 1e-12);
    EXPECT_NEAR(pixArray[1], m(1), 1e-12);

    EXPECT_NEAR(rayPArray[0], rayP(0), 1e-5);
    EXPECT_NEAR(rayPArray[1], rayP(1), 1e-5);
    EXPECT_NEAR(rayPArray[2], rayP(2), 1e-5);

    EXPECT_NEAR(ray(0), rayP(0), 1e-12);
    EXPECT_NEAR(ray(1), rayP(1), 1e-12);
    EXPECT_NEAR(ray(2), rayP(2), 1e-12);
}

TEST(KannalaBrandtRadialDistortionModel4, realCoefficientUnprojectProject) {

    const Eigen::Matrix<double, 2, 1> pix = { 300, 300 };
    Eigen::Matrix<double, 3, 3> K;
    K << 771.4914, 0, 1244.208, 0, 770.1611, 1077.472, 0, 0, 1;

    Eigen::Matrix<double, 4, 1> coef;

    // Real coefficients from a fish-eye calibration
    coef(0) = 0.01117972;
    coef(1) = 0.04504434;
    coef(2) = -0.05763411;
    coef(3) = 0.02156141;

    KannalaBrandtRadialDistortionModel4<double> lensModel(K, coef, 100, 0);
    const Eigen::Matrix<double, 3, 1> ray = lensModel.unproject(pix);
    const Eigen::Matrix<double, 2, 1> pixP = lensModel.project(ray);

    EXPECT_NEAR(pix(0), pixP(0), 1e-12);
    EXPECT_NEAR(pix(1), pixP(1), 1e-12);
}

TEST(KannalaBrandtRadialDistortionModel4, centerPoint) {

    const Eigen::Matrix<double, 3, 1> ray = { 0, 0, 1 };
    const Eigen::Matrix<double, 3, 3> K = Eigen::Matrix<double, 3, 3>::Identity();

    Eigen::Matrix<double, 4, 1> coef;

    // Real coefficients from a fish-eye calibration
    coef(0) = 0.01117972;
    coef(1) = 0.04504434;
    coef(2) = -0.05763411;
    coef(3) = 0.02156141;

    KannalaBrandtRadialDistortionModel4<double> lensModel(K, coef, 100, 0);
    const Eigen::Matrix<double, 2, 1> m = lensModel.project(ray);
    const Eigen::Matrix<double, 3, 1> rayP = lensModel.unproject(m);

    EXPECT_DOUBLE_EQ(ray(0), rayP(0));
    EXPECT_DOUBLE_EQ(ray(1), rayP(1));
    EXPECT_DOUBLE_EQ(ray(2), rayP(2));
}
