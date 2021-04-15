#include "events.h"

#include "command_handler.h"
#include "comm_ascii.h"
#include "comm_protobuf.h"
#include "stm32f4xx_hal.h"

CommandHandler *command_handler = nullptr;
volatile core_SystemTimestamp current_timestamp = {0};

void OnInitialization(struct CommandHandlerConfiguration const *config) {
	static uint8_t command_handler_allocation[sizeof(CommandHandler)];
	if (command_handler) {
		command_handler->~CommandHandler();
	}
	command_handler = new (command_handler_allocation) CommandHandler(*config);
}

void OnHallSensorTransitionInterrupt() {
	if (command_handler != nullptr) {
		command_handler->OnHallSensorTransitionInterrupt();
	}
}

void OnProcessorTick() {
	if (current_timestamp.nanos != 0) {
		current_timestamp.nanos += 1000000;
	}
	if (command_handler != nullptr) {
		command_handler->OnProcessorTick();
	}
}

void OnStartup() {
	// On startup, we try to see if the servos are in a sane state.

	// Or, uncomment this to try to set all servos to ID 1 to get to a known state!
	//DynamixelPacket::CreateWrite(0xFE, false, DynamixelPacket::ADDR_ID, 1).SendToServo();

	// First, turn on the LEDs on all of them so that we can see what's connected.
	DynamixelPacket::CreateWrite(0xFE, false, DynamixelPacket::ADDR_LED, 1).SendToServo();

	// Next check if required servos exists. We can do this with multicast ping, but unicast lets us avoid
	// multiple responses. And just turning off its LED is both practical and already implemented.
	uint8_t expected_servos[] = {2, 3};
	uint8_t expected_servo_flash_codes[sizeof(expected_servos)/sizeof(*expected_servos)] = {0x0A, 0x2A};
	for (size_t i = 0; i < sizeof(expected_servos)/sizeof(*expected_servos); i++) {
		bool has_expected_servo = DynamixelPacket::CreateWrite(expected_servos[i], false, DynamixelPacket::ADDR_LED, 0).SendToServo();

		// No servo?  That's an error -- see if there's a servo 1 that we can make servo N!
		// We flash the blue light with the motor code we're proposing, and flash the servo to match.
		// If the user accepts, they can press the blue button, at which point we configure servo 1 into servo 2
		// and continue with boot. If something goes wrong, we lock the blue LED on and freeze.
		uint8_t flash_code = expected_servo_flash_codes[i];
		bool has_servo_1 = true;
		while (!has_expected_servo) {
			bool now_has_servo_1 = DynamixelPacket::CreateWrite(1, false, DynamixelPacket::ADDR_LED, 0).SendToServo();
			if (now_has_servo_1 != has_servo_1) {
				has_servo_1 = now_has_servo_1;
				flash_code = has_servo_1 ? expected_servo_flash_codes[i] : 0xAA;
			}

			HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, (flash_code & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
			if (has_servo_1) {
				DynamixelPacket::CreateWrite(1, false, DynamixelPacket::ADDR_LED, flash_code & 1).SendToServo();
			}
			flash_code = static_cast<uint8_t>((flash_code << 1) | (flash_code >> 7));

			if (has_servo_1 && HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin)) {
				bool change_okay = DynamixelPacket::CreateWrite(1, false, DynamixelPacket::ADDR_ID, expected_servos[i]).SendToServo();
				if (!change_okay) {
					HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_SET);
					for(;;);
				}
				break;
			}

			has_expected_servo = DynamixelPacket::CreateWrite(expected_servos[i], false, DynamixelPacket::ADDR_LED, 0).SendToServo();
		}
	}

	// Once we have all the servos, turn on torque and center them!
	for (size_t i = 0; i < sizeof(expected_servos)/sizeof(*expected_servos); i++) {
		bool torque_okay = DynamixelPacket::CreateWrite(
				expected_servos[i], false, DynamixelPacket::ADDR_TORQUE_ENABLE, 1).SendToServo();
		if (!torque_okay) {
			HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_SET);
			for(;;);
		}
	}
	DynamixelPacket::CreateWrite(0xFE, false, DynamixelPacket::ADDR_GOAL_POSITION, SteeringServo::HOME_POSITION).SendToServo();
	HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_RESET);
}

#if USE_ASCII_INSTEAD_OF_PROTOS
void OnMainLoop() { OnMainLoop_ASCII(); }
void OnUSBReceive(uint8_t* Buf, uint32_t *Len) { OnUSBReceive_ASCII(Buf, Len); }
#else
void OnMainLoop() { OnMainLoop_Protobuf(); }
void OnUSBReceive(uint8_t* Buf, uint32_t *Len) { OnUSBReceive_Protobuf(Buf, Len); }
#endif
