/*
 * comm_ascii.cpp
 *
 *  Created on: May 24, 2017
 *      Author: addaon
 */

#include "comm_ascii.h"

#include "command_handler.h"
#include "motor.h"
#include "usbd_cdc_if.h"
#include "packages/hal/proto/vcu_command_envelope.nanopb.h"
#include "packages/hal/proto/vcu_command_response.nanopb.h"

static volatile int has_pending_error = 0;
static volatile int has_pending_command = 0;
static CommandHandler::MotionCommand pending_command;

void OnUSBReceive_ASCII(uint8_t* Buf, uint32_t *Len) {
	if (Len && *Len < std::numeric_limits<uint16_t>::max()) {
		HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
		CDC_Transmit_FS(Buf, static_cast<uint16_t>(*Len));  // Best effort, no checking
		HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);

		static enum {
			V_READY, VELOCITY, C_READY, CURVATURE, ERROR
		} state = V_READY;
		static float v_scale = 1.0f;
		static float v_value = 0.0f;
		static float c_scale = 1.0f;
		static float c_value = 0.0f;

		for (uint32_t i = 0; i < *Len; i++) {
			uint8_t c = Buf[i];
			switch (state) {
			case V_READY:
				switch (c) {
				case '+': v_scale = +1.0f; break;
				case '-': v_scale = -1.0f; break;
				case '0' ... '9': v_value += v_scale * static_cast<float>(c - '0'); state = VELOCITY; break;
				default: state = ERROR; --i; break;
				}
				break;
			case VELOCITY:
				switch (c) {
				case '0' ... '9':
					if (v_scale >= 1.0f) v_value *= 10.0f;
					v_value += v_scale * static_cast<float>(c - '0');
					if (v_scale < 1.0f) v_scale /= 10.0f;
					break;
				case '.':
					v_scale /= 10.0f;
					break;
				case ',': state = C_READY; break;
				default: state = ERROR; --i; break;
				}
				break;
			case C_READY:
				switch (c) {
				case '+': c_scale = +1.0f; break;
				case '-': c_scale = -1.0f; break;
				case '0' ... '9': c_value += c_scale * static_cast<float>(c - '0'); state = CURVATURE; break;
				default: state = ERROR; --i; break;
				}
				break;
			case CURVATURE:
				switch (c) {
				case '0' ... '9':
					if (c_scale >= 1.0f) c_value *= 10.0f;
					c_value += c_scale * static_cast<float>(c - '0');
					if (c_scale < 1.0f) c_scale /= 10.0f;
					break;
				case '.':
					c_scale /= 10.0f;
					break;
				case '\r':
					break;
				case '\n':
					has_pending_command = 1;
					pending_command = {v_value, c_value};
					state = V_READY;
					v_scale = 1.0f;
					v_value = 0.0f;
					c_scale = 1.0f;
					c_value = 0.0f;
					break;
				default: state = ERROR; --i; break;
				}
				break;
			case ERROR:
				switch (c) {
				case '\r':
					break;
				case '\n':
					++has_pending_error;
					state = V_READY;
					v_scale = 1.0f;
					v_value = 0.0f;
					c_scale = 1.0f;
					c_value = 0.0f;
					break;
				default:
					/* nothing */
					break;
				}
			}
		}
	}
}

void OnMainLoop_ASCII() {
	  int has_pending_error_shadow;
	  int has_pending_command_shadow;
	  struct CommandHandler::MotionCommand pending_command_shadow;

	  __disable_irq();
	  has_pending_error_shadow = has_pending_error;
	  has_pending_command_shadow = has_pending_command;
	  pending_command_shadow = pending_command;
	  has_pending_error = 0;
	  has_pending_command = 0;
	  __enable_irq();
	  if (has_pending_error_shadow) {
			uint8_t error_msg[] = "parse error\r\n";
			HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
			CDC_Transmit_FS(error_msg, sizeof(error_msg));  // Best effort, no checking
			HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
	  }
	  if (has_pending_command_shadow) {
		  	uint8_t init_msg[] = "init error\r\n";
		  	uint8_t ack_msg[] = "okay\r\n";
		  	uint8_t range_msg[] = "range error\r\n";
		  	uint8_t *msg = NULL;
		  	uint8_t msg_length = 0;

		  	if (!command_handler) {
		  		msg = init_msg;
		  		msg_length = sizeof(init_msg);
		  	} else if (pending_command_shadow.velocity_fwd_m_s >= -HARD_SPEED_LIMIT
		  			&& pending_command_shadow.velocity_fwd_m_s <= HARD_SPEED_LIMIT
					&& pending_command_shadow.curvature_right_inv_m >= -HARD_CURVATURE_LIMIT
					&& pending_command_shadow.curvature_right_inv_m <= HARD_CURVATURE_LIMIT)
		  	{
		  		command_handler->HandleCommand(pending_command_shadow);
	  		    msg = ack_msg;
	  		    msg_length = sizeof(ack_msg);
		  	} else {
		  		msg = range_msg;
		  		msg_length = sizeof(range_msg);
		  	}

			HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
			CDC_Transmit_FS(msg, msg_length);  // Best effort, no checking
			HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
	  }
}

