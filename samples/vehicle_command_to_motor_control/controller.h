#include "glog/logging.h"
#include <math.h>
#include <cstdio>

float getServoAngle(float xWheel, float yWheel, float centerCurvature)
{
    return atan(yWheel * centerCurvature / (1 - xWheel * centerCurvature));
}

void getMotorVelocity(float *vx, float *vy, float xWheel, float yWheel, float centerCurvature, float centerVelocity, float servoAngle)
{
    CHECK_NOTNULL(vx);
    CHECK_NOTNULL(vy);
    float rSqr_divide_rcSqr = ((centerCurvature * xWheel - 1) * (centerCurvature * xWheel - 1) + (centerCurvature * yWheel) * (centerCurvature * yWheel));
    float rest = - centerVelocity / ((centerCurvature * xWheel - 1) - centerCurvature * yWheel * tan(servoAngle));
    *vy = rSqr_divide_rcSqr * rest;
    *vx = (*vy) * tan(servoAngle);
}

float getMagnitude(float vx, float vy)
{
    return sqrt(vx * vx + vy * vy);
}

static constexpr int kForwardDirection = 1;
static constexpr int kBackwardDirection = -1;
int getDirection(float vy)
{
    return vy > 0 ? kForwardDirection : kBackwardDirection;
}
