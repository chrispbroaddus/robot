/*
 * comm_protobuf.cpp
 *
 *  Created on: May 24, 2017
 *      Author: addaon
 */

#include "comm_protobuf.h"

#include "events.h"
#include "motor.h"
#include "command_handler.h"
#include "usbd_cdc_if.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "packages/hal/proto/vcu_command_envelope.nanopb.h"
#include "packages/hal/proto/vcu_command_response.nanopb.h"

static void CommError(uint8_t flash_count) {
	// For now, hard fail (freeze and flash orange light) on any comm error.
	// Once we evaluate how we can safely handle message corruption, etc, we can revisit this.
	if (flash_count == 0 || flash_count > 12) {
		flash_count = 14;
	}
	uint32_t flash_code = 0;
	for (int i = 0; i < flash_count; i++) {
		flash_code <<= 2;
		flash_code |= 1;
	}
	for (;;) {
	  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, flash_code & 1 ? GPIO_PIN_SET : GPIO_PIN_RESET); // Orange flashing -> hard comm error
	  flash_code = (flash_code << 1) | (flash_code >> (8 * sizeof(flash_code) - 1));
	  for(size_t i = 0; i < 500000; i++);
	}
}

static constexpr size_t N_buffers = 2;
static constexpr size_t buffer_size = hal_VCUCommandEnvelope_size + 10;  // Longest proto + varint delimiter
static volatile size_t buffer_rx_idx = 0;
static struct {
	volatile size_t buffer_len = 0;
	uint8_t buffer[buffer_size] = {0};
} buffers[N_buffers];

void OnUSBReceive_Protobuf(uint8_t* Buf, uint32_t *Len) {

	auto &current_buffer = buffers[buffer_rx_idx];

	if (*Len > buffer_size || current_buffer.buffer_len + *Len >= buffer_size) {
		// Out of room in buffer, fail! Flashing orange LED for comm error
		CommError(1);
	}

	memcpy(&current_buffer.buffer[current_buffer.buffer_len], Buf, *Len);
	current_buffer.buffer_len += *Len;

	// If at any time the current buffer contains a valid varint, followed by that many bytes,
	// the protobuf is complete. We will flag it as such to hand it over to the main task for
	// processing and switch to using the other buffer; the main task must finish with its buffer
	// before we fill up the next one.

	pb_istream_t istream = pb_istream_from_buffer(current_buffer.buffer, current_buffer.buffer_len);
	uint32_t varint_value = 0;
	if (pb_decode_varint32(&istream, &varint_value) && istream.bytes_left >= varint_value) {
		size_t next_buffer_idx = (buffer_rx_idx + 1) % N_buffers;
		auto &next_buffer = buffers[next_buffer_idx];

		if (next_buffer.buffer_len) {
			// Main task did not complete emptying the other buffer, and we need it now!
			CommError(2);
		}

		size_t extra_bytes = istream.bytes_left - varint_value;
		memcpy(&next_buffer.buffer[next_buffer.buffer_len], &current_buffer.buffer[current_buffer.buffer_len - extra_bytes], extra_bytes);
		current_buffer.buffer_len -= extra_bytes;
		next_buffer.buffer_len += extra_bytes;

		buffer_rx_idx = next_buffer_idx;
	}
}

void OnMainLoop_Protobuf() {

	static hal_VCUCommandEnvelope current_command = {0, hal_VCUCommandEnvelope_idleCommand_tag, {}};
	static CommandHandler::MotionCommand last_command_sent = {0.0f, 0.0f};
	static core_SystemTimestamp __attribute__((unused)) time_last_command_sent = {0};  // Unused when not lerping, suppress warning

	// See if new command has come in from OnUSBReceive
	size_t buffer_idx = (buffer_rx_idx - 1) % N_buffers;  // Look at the previous index
	auto &current_buffer = buffers[buffer_idx];
	if (current_buffer.buffer_len) {
		// We received something, process it and mark it as done by clearing length!
		pb_istream_t istream = pb_istream_from_buffer(current_buffer.buffer, current_buffer.buffer_len);

		hal_VCUCommandEnvelope decoded;
		if (!pb_decode_delimited(&istream, hal_VCUCommandEnvelope_fields, &decoded)) {
			// Failure to decode!
			CommError(3);
		}

		if (istream.bytes_left != 0) {
			// Unexpected message length!
			CommError(4);
		}

		current_buffer.buffer_len = 0;

		// Now validate the decoded protobuf and send an ack.
		bool valid = true;
		uint32_t error_reason = 0;
		if (!command_handler) {
			// We're not properly initialized!
			valid = false;
			error_reason = 1;
		}
		if (decoded.sequenceNumber == 0) {
			// Sequence number 0 is reserved for VCU use
			valid = false;
			error_reason = 2;
		}
		switch (decoded.which_command) {
		case hal_VCUCommandEnvelope_idleCommand_tag: {
			// No checking needed
			break;
		}
		case hal_VCUCommandEnvelope_emergencyStopCommand_tag: {
			// TODO: Handle this command!
			valid = false;
			error_reason = 3;
			break;
		}
		case hal_VCUCommandEnvelope_trajectoryCommand_tag: {
			auto const &trajectory = decoded.command.trajectoryCommand;
			// Check 1: There are a reasonable number of segments;
			if (trajectory.segments_count < 1 || trajectory.segments_count >= sizeof(trajectory.segments)/sizeof(trajectory.segments[0])) {
				valid = false;
				error_reason = 4;
				break;
			}
			// Check 2: The trajectory terminates at zero velocity:
			if (trajectory.segments[trajectory.segments_count - 1].linearVelocityMetersPerSecond != 0.0) {
				valid = false;
				error_reason = 5;
				break;
			}
			// Check 3: All segment velocities and curvatures are in range:
			for (size_t i = 0; i < trajectory.segments_count; i++) {
				if (trajectory.segments[i].linearVelocityMetersPerSecond < -HARD_SPEED_LIMIT
			     || trajectory.segments[i].linearVelocityMetersPerSecond > +HARD_SPEED_LIMIT
				 || trajectory.segments[i].curvatureInverseMeters < -HARD_CURVATURE_LIMIT
				 || trajectory.segments[i].curvatureInverseMeters > +HARD_CURVATURE_LIMIT) {
					valid = false;
					error_reason = 6;
					break;
				}
			}
			if (valid == false) {
				break;
			}
			// Check 4: The first segment is in the future:
			if (trajectory.segments[0].targetStartTime.nanos <= current_timestamp.nanos) {
				valid = false;
				error_reason = 7;
				break;
			}
			// Check 5: The trajectory segments are in chronological order:
			for (size_t i = 1; i < trajectory.segments_count; i++) {
				if (trajectory.segments[i].targetStartTime.nanos <= trajectory.segments[i-1].targetStartTime.nanos) {
					valid = false;
					error_reason = 8;
					break;
				}
			}
			if (valid == false) {
				break;
			}
			// We made it!
			break;
		}
		default: {
			// Unknown command tag!
			valid = false;
			error_reason = 9;
			break;
		}
		}

		if (error_reason && valid) {
			valid = false;
			error_reason = 10;
		}

		// Build a response packet
		static hal_VCUCommandResponse response;  // Static due to size, this function not re-entrant
		memset(&response, 0, sizeof(response));
		response.disposition = valid ? hal_VCUCommandDisposition_CommandAccepted : hal_VCUCommandDisposition_CommandRejected;
		response.timestamp = {current_timestamp.nanos};
		response.error_reason = error_reason;
		if (DEBUG_SEND_FULL_COMMANDS_IN_RESPONSES) {
			response.which_source = hal_VCUCommandResponse_command_tag;
			response.source.command = decoded;
		} else {
			response.which_source = hal_VCUCommandResponse_sequenceNumber_tag;
			response.source.sequenceNumber = decoded.sequenceNumber;
		}
		if (!valid) {
			response.which_previous = hal_VCUCommandResponse_retainedCommand_tag;
			response.previous.retainedCommand = current_command;
		} else if(DEBUG_SEND_FULL_COMMANDS_IN_RESPONSES) {
			response.which_previous = hal_VCUCommandResponse_previousCommand_tag;
			response.previous.previousCommand = current_command;
		} else {
			response.which_previous = hal_VCUCommandResponse_previousSequenceNumber_tag;
			response.previous.previousSequenceNumber = current_command.sequenceNumber;
		}

		// Remember the new command if we're accepting it
		if (valid) {
			current_command = decoded;
#if DEBUG_TIME_IS_ASSUMED_TO_START_WITH_FIRST_TRAJECTORY
			if (current_timestamp.nanos == 0 && current_command.which_command == hal_VCUCommandEnvelope_trajectoryCommand_tag) {
				current_timestamp.nanos = current_command.command.trajectoryCommand.segments[0].targetStartTime.nanos;
			}
#endif
		}

		// Turn off orange LED during ack send
		static uint8_t obuf[hal_VCUCommandResponse_size + 10] = {0};  // Static due to size, this function not re-entrant
		pb_ostream_t ostream = pb_ostream_from_buffer(obuf, sizeof(obuf)/sizeof(*obuf));
		if (!pb_encode_delimited(&ostream, hal_VCUCommandResponse_fields, &response)) {
			// Failure to encode!
			CommError(5);
		}
		if (ostream.bytes_written == 0 || ostream.bytes_written > sizeof(obuf)/sizeof(*obuf)) {
			CommError(6);
		}

		// Throw the now-encoded ack over USB
		uint8_t result = USBD_BUSY;
		do {
			result = CDC_Transmit_FS(obuf, static_cast<uint16_t>(ostream.bytes_written));
			if (result == USBD_FAIL) {
				CommError(7);
			}
		} while (result == USBD_BUSY);
		HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
	}

	// Process the current command if necessary
	bool send_updated_command = false;
	bool send_completion_packet = false;
	switch (current_command.which_command) {
	case hal_VCUCommandEnvelope_idleCommand_tag: {
		if (last_command_sent.velocity_fwd_m_s != 0.0f || last_command_sent.curvature_right_inv_m != 0.0f) {
			last_command_sent = {0.0f, 0.0f};
			send_updated_command = true;
		}
		break;
	}
	case hal_VCUCommandEnvelope_emergencyStopCommand_tag: {
		// TODO: Handle this command!
		break;
	}
	case hal_VCUCommandEnvelope_trajectoryCommand_tag: {
		auto const &trajectory = current_command.command.trajectoryCommand;
		// Find the first trajectory segment that ends after the current time
		bool send_completion_packet = true;
		size_t segment_index = 0;
		for (; segment_index < trajectory.segments_count; segment_index++) {
			if (trajectory.segments[segment_index].targetStartTime.nanos >= current_timestamp.nanos) {
				send_completion_packet = false;
				break;
			}
		}
		if (!send_completion_packet) {
#if DO_INTERPOLATION_OF_TRAJECTORIES
			// Lerp between last sent command and current one
			float ratio = static_cast<float>(current_timestamp.nanos - time_last_command_sent.nanos) /
					      static_cast<float>(trajectory.segments[segment_index].targetStartTime.nanos - time_last_command_sent.nanos);
			CommandHandler::MotionCommand lerped = {
					last_command_sent.velocity_fwd_m_s * (1.0f - ratio) + trajectory.segments[segment_index].linearVelocityMetersPerSecond * ratio,
					last_command_sent.curvature_right_inv_m * (1.0f - ratio) + trajectory.segments[segment_index].curvatureInverseMeters * ratio};
			last_command_sent = lerped;
			send_updated_command = true;
#else
			// Just execute the newest command
			auto const &segment = trajectory.segments[segment_index ? segment_index - 1 : 0];
			if (segment.linearVelocityMetersPerSecond != last_command_sent.velocity_fwd_m_s || segment.curvatureInverseMeters != last_command_sent.curvature_right_inv_m) {
				last_command_sent = {segment.linearVelocityMetersPerSecond, segment.curvatureInverseMeters};
				send_updated_command = true;
			}

#endif
		}
		break;
	}
	}
	if (send_completion_packet) {
		// Send the completion packet. In a completion packet, the "source" is our own idle packet,
		// and the "previous command" is the one that just completed.

#if !DEBUG_DONT_SEND_COMPLETIONS
		// Synthesize the message
		static hal_VCUCommandResponse response;  // Static due to size, this function not re-entrant
		memset(&response, 0, sizeof(response));
		response.disposition = hal_VCUCommandDisposition_CommandCompleted;
		response.timestamp = {current_timestamp.nanos};
		response.which_source = hal_VCUCommandResponse_sequenceNumber_tag;
		response.source.sequenceNumber = 0;
		if(DEBUG_SEND_FULL_COMMANDS_IN_RESPONSES) {
			response.which_previous = hal_VCUCommandResponse_previousCommand_tag;
			response.previous.previousCommand = current_command;
		} else {
			response.which_previous = hal_VCUCommandResponse_previousSequenceNumber_tag;
			response.previous.previousSequenceNumber = current_command.sequenceNumber;
		}

		// Turn off orange LED during ack send
		static uint8_t obuf[hal_VCUCommandResponse_size + 10] = {0};  // Static due to size, this function not re-entrant
		pb_ostream_t ostream = pb_ostream_from_buffer(obuf, sizeof(obuf)/sizeof(*obuf));
		if (!pb_encode_delimited(&ostream, hal_VCUCommandResponse_fields, &response)) {
			// Failure to encode!
			CommError(8);
		}
		if (ostream.bytes_written == 0 || ostream.bytes_written > sizeof(obuf)/sizeof(*obuf)) {
			CommError(9);
		}

		// Throw the now-encoded ack over USB
		uint8_t result = USBD_BUSY;
		do {
			result = CDC_Transmit_FS(obuf, static_cast<uint16_t>(ostream.bytes_written));
			if (result == USBD_FAIL) {
				CommError(10);
			}
		} while (result == USBD_BUSY);
		HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
#endif

		// Synthesize an idle command
		current_command = {0, hal_VCUCommandEnvelope_idleCommand_tag, {}};
		last_command_sent = {0.0f, 0.0f};
		send_updated_command = true;
	}
	if (send_updated_command) {
		if (command_handler) {
			// Should always be true due to earlier check
			command_handler->HandleCommand(last_command_sent);
		}
		time_last_command_sent.nanos = current_timestamp.nanos;
	}
}


