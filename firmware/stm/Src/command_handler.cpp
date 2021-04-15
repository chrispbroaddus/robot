/*
 * command_handler.cpp
 *
 *  Created on: May 24, 2017
 *      Author: addaon
 */

#include "command_handler.h"

#include "events.h"
#include "main.h"
#include "usbd_cdc_if.h"

void CommandHandler::HandleCommand(MotionCommand const &cmd) {
	for (size_t i = 0; i < N_MOTORS; i++) {

		float radius_vehicle = 1.0f / cmd.curvature_right_inv_m;
		float theta_wheel = std::atan2(configuration.motors[i].offset_y, radius_vehicle - configuration.motors[i].offset_x);
		if (theta_wheel <= -M_PIf/2.0f) {
			theta_wheel += M_PIf;
			radius_vehicle *= -1.0f;
		} else if (theta_wheel > M_PIf/2.0f) {
			theta_wheel -= M_PIf;
			radius_vehicle *= -1.0f;
		}
		float velocity_wheel = cmd.velocity_fwd_m_s *
				(1.0f - configuration.motors[i].offset_x * cmd.curvature_right_inv_m) / std::cos(theta_wheel);
#if SKID_STEERING
		velocity_wheel *= std::cos(theta_wheel);
#endif

		long velocity_hall_s = std::lround(
			velocity_wheel * HALL_TRANSITIONS_PER_REVOLUTION / WHEEL_CIRCUMFERENCE_m);
		if (configuration.motors[i].reverse_direction) {
			velocity_hall_s = -velocity_hall_s;
		}

		motors[i].SetVelocity( velocity_hall_s > Motor::MAX_VELOCITY ? Motor::MAX_VELOCITY
					 : velocity_hall_s < -Motor::MAX_VELOCITY ? -Motor::MAX_VELOCITY
					 : static_cast<int32_t>(velocity_hall_s));

		steering_servos[i].RegisterSetAngle(theta_wheel);
	}
	SteeringServo::CommitSetAngles();
}

void CommandHandler::OnHallSensorTransitionInterrupt() {
	for (size_t i = 0; i < N_MOTORS; i++) {
		motors[i].OnHallSensorTransitionInterrupt();
	}
}

void CommandHandler::OnProcessorTick() {
	for (size_t i = 0; i < N_MOTORS; i++) {
		motors[i].OnProcessorTick();
	}
}

