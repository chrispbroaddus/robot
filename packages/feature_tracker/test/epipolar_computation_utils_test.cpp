#include "packages/feature_tracker/include/epipolar_computation_utils.h"

#include "gtest/gtest.h"

using namespace feature_tracker;

TEST(EpipolarComputationTest, essentialMatrixComputationTest) {

    Eigen::Matrix<float, 3, 3> rotationMatrix;
    Eigen::Matrix<float, 3, 1> translationVector(1, 2, 3);
    rotationMatrix << 1, 2, 9, 4, 5, 6, 7, 8, 3;

    Eigen::Matrix<float, 3, 3> essentialMatrix = computeEssentialMatrix(rotationMatrix, translationVector);

    EXPECT_NEAR(2, essentialMatrix(0, 0), 1.0e-6f);
    EXPECT_NEAR(1, essentialMatrix(0, 1), 1.0e-6f);
    EXPECT_NEAR(-12, essentialMatrix(0, 2), 1.0e-6f);
    EXPECT_NEAR(-4, essentialMatrix(1, 0), 1.0e-6f);
    EXPECT_NEAR(-2, essentialMatrix(1, 1), 1.0e-6f);
    EXPECT_NEAR(24, essentialMatrix(1, 2), 1.0e-6f);
    EXPECT_NEAR(2, essentialMatrix(2, 0), 1.0e-6f);
    EXPECT_NEAR(1, essentialMatrix(2, 1), 1.0e-6f);
    EXPECT_NEAR(-12, essentialMatrix(2, 2), 1.0e-6f);
}

TEST(EpipolarComputationTest, shortestDistFromPointToLine_badPoint) {

    Eigen::Matrix<float, 3, 1> point(2, 3, 0.9f);
    Eigen::Matrix<float, 3, 1> line(-1, 1, -1);

    EXPECT_THROW(shortestDistPointToLine(point, line), std::runtime_error);
}

TEST(EpipolarComputationTest, shortestDistFromPointToLine) {

    Eigen::Matrix<float, 3, 1> point(2, 4, 1);
    Eigen::Matrix<float, 3, 1> line(-1, 1, -1);

    EXPECT_NEAR(1.0f / std::sqrt(2.0f), shortestDistPointToLine(point, line), 1.0e-6f);
}

TEST(EpipolarComputationTest, shortestDistFromPointToLine_pointOnLine) {

    Eigen::Matrix<float, 3, 1> point(2, 3, 1);
    Eigen::Matrix<float, 3, 1> line(-1, 1, -1);

    EXPECT_NEAR(0, shortestDistPointToLine(point, line), 1.0e-6f);
}

TEST(EpipolarComputationTest, computeRightEpipolarLine_NoRot_XTranslation) {

    Eigen::Matrix<float, 3, 3> rotationMatrix;
    Eigen::Matrix<float, 3, 1> translationVector(1, 0, 0);
    rotationMatrix << 1, 0, 0, 0, 1, 0, 0, 0, 1;

    Eigen::Matrix<float, 3, 3> essentialMatrix = computeEssentialMatrix(rotationMatrix, translationVector);
    Eigen::Matrix<float, 3, 1> leftImagePoint(0.6f, 0.8f, 1.f);
    Eigen::Matrix<float, 3, 1> rightImagePoint(0.4f, 0.8f, 1.f);

    Eigen::Matrix<float, 3, 1> rightEpipolarLine = computeRightEpipolarLine(essentialMatrix, leftImagePoint);

    EXPECT_NEAR(0.f, rightEpipolarLine(0, 0), 1.0e-6f);
    EXPECT_NEAR(1.f, rightEpipolarLine(1, 0), 1.0e-6f);
    EXPECT_NEAR(-0.8f, rightEpipolarLine(2, 0), 1.0e-6f);

    EXPECT_NEAR(0.f, shortestDistPointToLine(rightImagePoint, rightEpipolarLine), 1.0e-5f);
}

TEST(EpipolarComputationTest, computeLeftEpipolarLine_NoRot_XTranslation) {

    Eigen::Matrix<float, 3, 3> rotationMatrix;
    Eigen::Matrix<float, 3, 1> translationVector(1, 0, 0);
    rotationMatrix << 1, 0, 0, 0, 1, 0, 0, 0, 1;

    Eigen::Matrix<float, 3, 3> essentialMatrix = computeEssentialMatrix(rotationMatrix, translationVector);
    Eigen::Matrix<float, 3, 1> leftImagePoint(0.6, 0.8, 1);
    Eigen::Matrix<float, 3, 1> rightImagePoint(0.4, 0.8, 1);

    Eigen::Matrix<float, 3, 1> leftEpipolarLine = computeLeftEpipolarLine(essentialMatrix, rightImagePoint);

    EXPECT_NEAR(0.f, leftEpipolarLine(0, 0), 1.0e-6f);
    EXPECT_NEAR(-1.f, leftEpipolarLine(1, 0), 1.0e-6f);
    EXPECT_NEAR(0.8f, leftEpipolarLine(2, 0), 1.0e-6f);

    EXPECT_NEAR(0.f, shortestDistPointToLine(leftImagePoint, leftEpipolarLine), 1.0e-5f);
}

TEST(EpipolarComputationTest, computeLeftEpipolarLine_NoRot_XYZTranslation) {

    Eigen::Matrix<float, 3, 3> rotationMatrix;
    Eigen::Matrix<float, 3, 1> translationVector(3, 2, 1);
    rotationMatrix << 1, 0, 0, 0, 1, 0, 0, 0, 1;

    Eigen::Matrix<float, 3, 3> essentialMatrix = computeEssentialMatrix(rotationMatrix, translationVector);
    Eigen::Matrix<float, 3, 1> rightImagePoint(-0.333333, 0.333333, 1);
    Eigen::Matrix<float, 3, 1> leftImagePoint(0.5, 0.75, 1);

    Eigen::Matrix<float, 3, 1> leftEpipolarLine = computeLeftEpipolarLine(essentialMatrix, rightImagePoint);

    EXPECT_NEAR(1.666667, leftEpipolarLine(0, 0), 1.0e-6);
    EXPECT_NEAR(-3.333333, leftEpipolarLine(1, 0), 1.0e-6);
    EXPECT_NEAR(1.666665, leftEpipolarLine(2, 0), 1.0e-6);

    EXPECT_NEAR(0, shortestDistPointToLine(leftImagePoint, leftEpipolarLine), 1.0e-5);
}

TEST(EpipolarComputationTest, computeLeftEpipolarLine_singleAxisRot_XTranslation) {

    Eigen::Matrix<float, 3, 3> rot1;
    float angle1 = (float)(30.0 * M_PI / 180.0);
    rot1 << cos(angle1), -sin(angle1), 0, sin(angle1), cos(angle1), 0, 0, 0, 1;
    Eigen::Matrix<float, 3, 3> rot2;
    float angle2 = (float)(45.0 * M_PI / 180.0);
    rot2 << cos(angle2), 0, -sin(angle2), 0, 1, 0, sin(angle2), 0, cos(angle2);

    Eigen::Matrix<float, 3, 3> rotationMatrix;
    Eigen::Matrix<float, 3, 1> translationVector(2, 0, 0);
    rotationMatrix << 0, -1, 0, 1, 0, 0, 0, 0, 1;

    Eigen::Matrix<float, 3, 3> essentialMatrix = computeEssentialMatrix(rotationMatrix, translationVector);
    Eigen::Matrix<float, 3, 1> rightImagePoint(2, -2.8, 1);
    Eigen::Matrix<float, 3, 1> leftImagePoint(3, 2, 1);

    Eigen::Matrix<float, 3, 1> leftEpipolarLine = computeLeftEpipolarLine(essentialMatrix, rightImagePoint);

    EXPECT_NEAR(0, leftEpipolarLine(0, 0), 1.0e-6);
    EXPECT_NEAR(-2, leftEpipolarLine(1, 0), 1.0e-6);
    EXPECT_NEAR(4, leftEpipolarLine(2, 0), 1.0e-6);

    EXPECT_NEAR(0, shortestDistPointToLine(leftImagePoint, leftEpipolarLine), 1.0e-5);
}

TEST(EpipolarComputationTest, computeLeftEpipolarLine_2AxisRot_XYZTranslation) {

    Eigen::Matrix<float, 3, 3> rotationMatrix;
    Eigen::Matrix<float, 3, 1> translationVector(2, 1, 3);
    rotationMatrix << 0.612372, -0.5, -0.612372, 0.353553, 0.866025, -0.353553, 0.707107, 0, 0.707107;

    Eigen::Matrix<float, 3, 3> essentialMatrix = computeEssentialMatrix(rotationMatrix, translationVector);

    Eigen::Matrix<float, 3, 1> point3D(30, 20, 10);
    Eigen::Matrix<float, 3, 1> rightImagePoint;
    Eigen::Matrix<float, 3, 1> leftImagePoint = point3D;
    leftImagePoint *= 1.0f / leftImagePoint(2, 0);
    rightImagePoint = rotationMatrix.transpose() * point3D - rotationMatrix.transpose() * translationVector;
    rightImagePoint *= 1.0f / rightImagePoint(2, 0);

    Eigen::Matrix<float, 3, 1> leftEpipolarLine = computeLeftEpipolarLine(essentialMatrix, rightImagePoint);

    EXPECT_NEAR(0, shortestDistPointToLine(leftImagePoint, leftEpipolarLine), 1.0e-5);
}
