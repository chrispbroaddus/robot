#include "samples/vehicle_command_to_motor_control/controller.h"

int main() {

    // Example of Trajectory command
    // valid (safe) centerCurvature range : -1/xWheelLefts < range < 1/xWheelRights
    float centerCurvature = 1 / 15.f;
    float centerVelocity = 3.f;

    // Example of vehicle configuration
    float xWheelFrontRight = 0.3;
    float yWheelFrontRight = 0.4;

    //////////////////////////////////////////////////
    /// Get the data to control a FrontRight motor
    //////////////////////////////////////////////////
    float servoAngle = getServoAngle(xWheelFrontRight, yWheelFrontRight, centerCurvature);

    float velocityX;
    float velocityY;
    getMotorVelocity(&velocityX, &velocityY, xWheelFrontRight, yWheelFrontRight, centerCurvature, centerVelocity, servoAngle);
    float motorVelocity = getMagnitude(velocityX, velocityY);
    int velocityDirection = getDirection(velocityY);

    fprintf(stdout, "servoAngle : %f\n", servoAngle);
    fprintf(stdout, "velocityX : %f\n", velocityX);
    fprintf(stdout, "velocityY : %f\n", velocityY);
    fprintf(stdout, "motorVelocity : %f\n", motorVelocity);
    fprintf(stdout, "velocityDirection : %d\n", velocityDirection);
}
