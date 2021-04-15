#include "glog/logging.h"
#include "samples/vehicle_command_to_motor_control/controller.h"
#include "gtest/gtest.h"
#include <vector>

///
/// When the curvature is 0, the vehicle is in a linear motion.
/// Thus, all the wheel should have 0 angle, and same linear velocity magnitude
///
TEST(VehicleCommandMotorController, linear_motion) {
    // Example of Trajectory command
    // valid (safe) centerCurvature range : -1/xWheelLefts < range < 1/xWheelRights
    float centerCurvature = 0;
    float centerVelocity = 3.f;

    // Example of vehicle configuration
    std::vector<float> xWheels{ -0.3, 0.3 };
    std::vector<float> yWheels{ -0.4, 0, 0.4 };

    for (const auto xWheel : xWheels) {
        for (const auto yWheel : yWheels) {
            float servoAngle = getServoAngle(xWheel, yWheel, centerCurvature);

            float velocityX;
            float velocityY;
            getMotorVelocity(&velocityX, &velocityY, xWheel, yWheel, centerCurvature, centerVelocity, servoAngle);
            float motorVelocity = getMagnitude(velocityX, velocityY);
            int velocityDirection = getDirection(velocityY);

            EXPECT_NEAR(servoAngle, 0, 1e-5);
            EXPECT_EQ(velocityDirection, 1);
            EXPECT_NEAR(motorVelocity, centerVelocity, 1e-5);
        }
    }
}

///
/// The the curvature center is located at the 2*middleWheel's_x_position,
/// the velocity should be a half of the vehicle center velocity
///
TEST(VehicleCommandMotorController, curvature_at_two_times_rightmiddlewheel_position) {
    // Example of Trajectory command
    // valid (safe) centerCurvature range : -1/xWheelLefts < range < 1/xWheelRights
    float centerCurvature = 1 / 0.6f;
    float centerVelocity = 3.f;

    // Example of vehicle configuration
    float xWheel = 0.3;
    float yWheel = 0;

    float servoAngle = getServoAngle(xWheel, yWheel, centerCurvature);

    float velocityX;
    float velocityY;
    getMotorVelocity(&velocityX, &velocityY, xWheel, yWheel, centerCurvature, centerVelocity, servoAngle);
    float motorVelocity = getMagnitude(velocityX, velocityY);
    int velocityDirection = getDirection(velocityY);

    EXPECT_EQ(servoAngle, 0);
    EXPECT_EQ(velocityDirection, 1);
    EXPECT_NEAR(motorVelocity, centerVelocity / 2, 1e-5);
}

///
/// The the curvature center is located at the 2*middleWheel's_x_position,
/// the velocity should be a half of the vehicle center velocity
///
TEST(VehicleCommandMotorController, curvature_at_two_times_leftmiddlewheel_position) {
    // Example of Trajectory command
    // valid (safe) centerCurvature range : -1/xWheelLefts < range < 1/xWheelRights
    float centerCurvature = -1 / 0.6f;
    float centerVelocity = 3.f;

    // Example of vehicle configuration
    float xWheel = -0.3;
    float yWheel = 0;

    float servoAngle = getServoAngle(xWheel, yWheel, centerCurvature);

    float velocityX;
    float velocityY;
    getMotorVelocity(&velocityX, &velocityY, xWheel, yWheel, centerCurvature, centerVelocity, servoAngle);
    float motorVelocity = getMagnitude(velocityX, velocityY);
    int velocityDirection = getDirection(velocityY);

    EXPECT_EQ(servoAngle, 0);
    EXPECT_EQ(velocityDirection, 1);
    EXPECT_NEAR(motorVelocity, centerVelocity / 2, 1e-5);
}

///
/// When the curvature center is closed to the vehicle, e.g, between center and right wheels,
/// the right wheels directions are supposed to be opposite
///
TEST(VehicleCommandMotorController, curvature_center_close_to_vehicle_body) {
    // Example of Trajectory command
    // valid (safe) centerCurvature range : -1/xWheelLefts < range < 1/xWheelRights
    float centerCurvature = 1 / 0.15f;
    float centerVelocity = 3.f;

    // Example of vehicle configuration
    float xWheel = 0.3;
    float yWheel = 0.4;

    float servoAngle = getServoAngle(xWheel, yWheel, centerCurvature);

    float velocityX;
    float velocityY;
    getMotorVelocity(&velocityX, &velocityY, xWheel, yWheel, centerCurvature, centerVelocity, servoAngle);
    float motorVelocity = getMagnitude(velocityX, velocityY);
    int velocityDirection = getDirection(velocityY);

    EXPECT_EQ(velocityDirection, -1);
}
