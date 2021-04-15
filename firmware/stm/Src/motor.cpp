/*
 * motor.cpp
 *
 *  Created on: May 4, 2017
 *      Author: addaon
 */

#include "motor.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>

void MotorVelocityState::RecordHallSensorTransition(HallSensorTransitionEvent event) {
	if (count != COUNT_ERROR) {
		switch (event) {
		case NO_TRANSITION: break;
		case MOTOR_WENT_FORWARD: ++count; break;
		case MOTOR_WENT_BACKWARD: --count; break;
		case ERROR_OCCURRED: /* fallthrough */
		default: count = COUNT_ERROR;
		}
	}
}

MotorVelocityState::UpdateResult MotorVelocityState::UpdateVelocityOnProcessorTick() {
	if (count == COUNT_ERROR) {
		if (error == NO_ERROR) {
			error = SOFT_ERROR;
		} else {
			error = HARD_ERROR;
		}
	} else {
		if (error == SOFT_ERROR) {
			error = NO_ERROR;
		}
		velocity -= window[window_idx];
		velocity += count;
		window[window_idx] = count;
		if (++window_idx >= TICKS_IN_WINDOW) {
			window_idx = 0;
		}
		if (velocity > MAX_VELOCITY || velocity < -MAX_VELOCITY) {
			error = HARD_ERROR;
		}
	}
	count = 0;

	return { error, error == HARD_ERROR ? 0 : velocity * 1000 / TICKS_IN_WINDOW };
}

void MotorPidController::SetSetpoint(int32_t new_setpoint) {
	setpoint = new_setpoint;
}

int64_t MotorPidController::ComputeNextOutput(int32_t current_value) {
	int32_t error = setpoint - current_value;

	integral += error;
	if (integral > MAX_INTEGRAL) {
		integral = MAX_INTEGRAL;
	}
	if (integral < -MAX_INTEGRAL) {
		integral = -MAX_INTEGRAL;
	}

	int32_t derivative = error - last_error;
	last_error = error;

	//Enable to show in-range status of a single connected motor
	//HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, error > -5 ? GPIO_PIN_SET : GPIO_PIN_RESET); // Orange
	//HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, error < 5 ? GPIO_PIN_SET : GPIO_PIN_RESET); // Blue

	return static_cast<int64_t>(Kp) * error
	     + static_cast<int64_t>(Ki) * integral
	     + static_cast<int64_t>(Kd) * derivative
	     + static_cast<int64_t>(Kf) * setpoint;
}

void Motor::SetVelocity(int32_t velocity) {
	if (!gpio_port_a) return;

	if (velocity < -MAX_VELOCITY || velocity > MAX_VELOCITY) {
		AbortOnHardError();  // Input already sanitized
	}
	pid_controller.SetSetpoint(velocity);
	mode = velocity < 0 ? Mode::BACKWARD
	     : velocity > 0 ? Mode::FORWARD
	     : Mode::STOPPED;
}

void Motor::OnHallSensorTransitionInterrupt() {
	if (!gpio_port_a) return;

	int new_hall = ReadHallSensors();
	if (!initialized) {
		hall = new_hall;
		initialized = true;
	}
	int next_forward, next_backward;
	std::tie(next_forward, next_backward) = NextHallStates();
	auto transition = new_hall < 1 || new_hall > 6 ? MotorVelocityState::ERROR_OCCURRED
			: new_hall == hall ? MotorVelocityState::NO_TRANSITION
			: new_hall == next_backward ? MotorVelocityState::MOTOR_WENT_BACKWARD
			: new_hall == next_forward ? MotorVelocityState::MOTOR_WENT_FORWARD
			: MotorVelocityState::ERROR_OCCURRED;
	velocity_state.RecordHallSensorTransition(transition);
	if (new_hall >= 1 && new_hall <= 6) {
		hall = new_hall;
	}
}

void Motor::OnProcessorTick() {
	if (!gpio_port_a) return;

	auto result = velocity_state.UpdateVelocityOnProcessorTick();
	switch (result.status) {
	case MotorVelocityState::NO_ERROR: break;
	case MotorVelocityState::SOFT_ERROR: ReportSoftError(); break;
	case MotorVelocityState::HARD_ERROR: /* fallthrough */
	default: AbortOnHardError();
	}

	int64_t control64 = pid_controller.ComputeNextOutput(result.velocity) >> THROTTLE_SCALE_BITS;
	int32_t control = control64 > THROTTLE_STEPS ? THROTTLE_STEPS
			: control64 < -THROTTLE_STEPS ? -THROTTLE_STEPS
			: static_cast<int32_t>(control64);

	bool should_brake = (mode == Mode::STOPPED)
			 || (mode == Mode::FORWARD && (control < 0 || result.velocity < 0))
			 || (mode == Mode::BACKWARD && (control > 0 || result.velocity > 0))
			 || HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);

	// See comments around definition of current_direction / direction_change_countdown
	bool direction = mode != Mode::BACKWARD;
	if (current_direction != direction) {
		direction_change_countdown = DIRECTION_COUNTDOWN_START;
		current_direction = direction;
	}
	if (direction_change_countdown > 0) {
		--direction_change_countdown;
		should_brake = true;
	}

	SetBrake(should_brake);
	SetDirection(direction);
	SetThrottle(should_brake ? 0 : std::abs(control));

	// Optionally, use green LED to indicate brake engaged
	// HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, should_brake ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Motor::ReportSoftError() {
	if (soft_error_count < std::numeric_limits<size_t>::max()) {
		++soft_error_count;
	}

	HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, GPIO_PIN_SET); // Red on -> soft error
}

void Motor::AbortOnHardError() {
	SetBrake(true);
	SetDirection(true);
	SetThrottle(0);

	for(;;) {
		  HAL_GPIO_TogglePin(LD5_GPIO_Port, LD5_Pin); // Red flashing -> hard error
		  for(size_t i = 0; i < 500000; i++) {
			  if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_SET) {
				  // TODO -- the idea here was to support recovery, but we need to clear the error condition
				  break;
			  }
		  }
	}
}

std::tuple<int, int> Motor::NextHallStates() {
	switch (hall) {
	case 1: return std::make_tuple(5, 3);
	case 2: return std::make_tuple(3, 6);
	case 3: return std::make_tuple(1, 2);
	case 4: return std::make_tuple(6, 5);
	case 5: return std::make_tuple(4, 1);
	case 6: return std::make_tuple(2, 4);
	default: return std::make_tuple(0, 0);
	}
}

int Motor::ReadHallSensors() {
	return (HAL_GPIO_ReadPin(gpio_port_a, gpio_pin_a))
		 | (HAL_GPIO_ReadPin(gpio_port_b, gpio_pin_b) << 1)
		 | (HAL_GPIO_ReadPin(gpio_port_c, gpio_pin_c) << 2);
}

void Motor::SetBrake(bool brake_on) {
	  HAL_GPIO_WritePin(gpio_port_brake, gpio_pin_brake, brake_on ? GPIO_PIN_SET : GPIO_PIN_RESET);

	  bool all_brakes = HAL_GPIO_ReadPin(BRAKE1_GPIO_Port, BRAKE1_Pin) == GPIO_PIN_SET
			  	     && HAL_GPIO_ReadPin(BRAKE2_GPIO_Port, BRAKE2_Pin) == GPIO_PIN_SET
					 && HAL_GPIO_ReadPin(BRAKE3_GPIO_Port, BRAKE3_Pin) == GPIO_PIN_SET;
	  HAL_GPIO_WritePin(BRAKEX_GPIO_Port, BRAKEX_Pin, all_brakes ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Motor::SetDirection(bool go_forward) {
	  HAL_GPIO_WritePin(gpio_port_dir, gpio_pin_dir, go_forward ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Motor::SetThrottle(int32_t control) {
	if (control < 0 || control > THROTTLE_STEPS) {
		AbortOnHardError();  // Input already sanitized
	}

	uint16_t value = static_cast<uint16_t>(control);
	TIM_OC_InitTypeDef sConfigOC;
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = value
					? value + (THROTTLE_MIN_mV * THROTTLE_STEPS_FULL_RANGE / 3300)
					: THROTTLE_OFF_mV * THROTTLE_STEPS_FULL_RANGE / 3300;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;

	extern TIM_HandleTypeDef htim1;
	HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, timer_channel);
	HAL_TIM_PWM_Start(&htim1, timer_channel);
}
