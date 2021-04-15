/*
 * command_handler.h
 *
 *  Created on: May 24, 2017
 *      Author: addaon
 */

#ifndef COMMAND_HANDLER_H_
#define COMMAND_HANDLER_H_

#include "stm32f4xx_hal.h"
#include "motor.h"
#include "servo.h"

class CommandHandler {
public:
	CommandHandler(CommandHandlerConfiguration const &config) : configuration{config} {}
	~CommandHandler() = default;
	CommandHandler(CommandHandler const &) = delete;
	CommandHandler &operator =(CommandHandler const &) = delete;

	struct MotionCommand {
		float velocity_fwd_m_s;
		float curvature_right_inv_m;
	};

	void HandleCommand(MotionCommand const &cmd);
	void OnHallSensorTransitionInterrupt();
	void OnProcessorTick();

private:

	constexpr static size_t N_MOTORS = 3;

	CommandHandlerConfiguration configuration;
	Motor motors[N_MOTORS] = {
			{FLIP_HALLS_MOTOR1, ENABLE_MOTOR1 ? HALL1A_GPIO_Port : nullptr, HALL1A_Pin,
					HALL1B_GPIO_Port, HALL1B_Pin, HALL1C_GPIO_Port, HALL1C_Pin,
					DIRECTION1_GPIO_Port, DIRECTION1_Pin, BRAKE1_GPIO_Port, BRAKE1_Pin, TIM_CHANNEL_1},
			{FLIP_HALLS_MOTOR2, ENABLE_MOTOR2 ? HALL2A_GPIO_Port : nullptr, HALL2A_Pin,
					HALL2B_GPIO_Port, HALL2B_Pin, HALL2C_GPIO_Port, HALL2C_Pin,
					DIRECTION2_GPIO_Port, DIRECTION2_Pin, BRAKE2_GPIO_Port, BRAKE2_Pin, TIM_CHANNEL_2},
			{FLIP_HALLS_MOTOR3, ENABLE_MOTOR3 ? HALL3A_GPIO_Port : nullptr, HALL3A_Pin,
					HALL3B_GPIO_Port, HALL3B_Pin, HALL3C_GPIO_Port, HALL3C_Pin,
					DIRECTION3_GPIO_Port, DIRECTION3_Pin, BRAKE3_GPIO_Port, BRAKE3_Pin, TIM_CHANNEL_3}};
	SteeringServo steering_servos[N_MOTORS] = {{2}, {0}, {3}};

	constexpr static int HALL_TRANSITIONS_PER_REVOLUTION = 60;
	constexpr static float WHEEL_CIRCUMFERENCE_m = 0.160f * M_PIf;
};

#endif /* COMMAND_HANDLER_H_ */
