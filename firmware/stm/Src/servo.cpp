/*
 * servo.cpp
 *
 *  Created on: May 24, 2017
 *      Author: addaon
 */

#include "servo.h"

#include "main.h"
#include "stm32f4xx_hal.h"

#include <algorithm>
#include <cassert>
#include <cmath>

DynamixelPacket DynamixelPacket::CreateWrite(uint8_t id, bool registered, Address address, uint32_t value) {
	size_t bytes = BytesForAddress(address);
	assert(bytes && bytes <= sizeof(value));

	size_t data_idx = 0;
	uint8_t data[sizeof(value)+2];
	data[data_idx++] = static_cast<uint8_t>(address & 0xFF);
	data[data_idx++] = static_cast<uint8_t>((address >> 8) & 0xFF);
	for (size_t i = 0; i < bytes; i++) {
		data[data_idx++] = static_cast<uint8_t>(value & 0xFF);
		value >>= 8;
	}

	return {id, registered ? INS_REG_WRITE : INS_WRITE, data, bytes + 2};
}

DynamixelPacket DynamixelPacket::CreateFactoryReset(uint8_t id, uint8_t param) {
	assert(param == 0x01 || param == 0x02);
	return {id, INS_FACTORY_RESET, &param, 1};
}

DynamixelPacket DynamixelPacket::CreateAction(uint8_t id) {
	return {id, INS_ACTION, nullptr, 0};
}

bool DynamixelPacket::SendToServo() {
	if (!ENABLE_SERVOS) return true;

	/* TODO: Do this without blocking, this is awful! */
	extern UART_HandleTypeDef huart5;
	HAL_GPIO_WritePin(UART5_GPIO_GPIO_Port, UART5_GPIO_Pin, GPIO_PIN_SET);
	HAL_UART_Transmit(&huart5, reinterpret_cast<uint8_t*>(this),
					  static_cast<uint16_t>(sizeof(header) + data_bytes), HAL_MAX_DELAY);
	HAL_GPIO_WritePin(UART5_GPIO_GPIO_Port, UART5_GPIO_Pin, GPIO_PIN_RESET);

	if (header.packet_id[0] == 0xFE) {
		return true;
	} else {
		uint8_t buffer[std::max(sizeof(header), MAX_N_DATA_BYTES)] alignas(uint32_t[]);
		if (HAL_UART_Receive(&huart5, buffer, sizeof(header), 100) != HAL_OK) {
			return false;
		}

		// TODO: Figure out why we're suddenly seeing packets come back with either an additional
		// '\0' on front, or with the first few bytes corrupt. For now, find the end of the header
		// and trust that, then deliberately don't check the header.
		int16_t offset = 0;
		while (buffer[offset] != 0xFD) {
			++offset;
		}
		offset = static_cast<int16_t>(offset - 2);
		if (offset < 0) {
			return false;
		}
		if (offset > 0) {
			if (HAL_UART_Receive(&huart5, buffer + sizeof(header), offset, HAL_MAX_DELAY) != HAL_OK) {
				return false;
			}
		}

		DynamixelPacketHeader response_header = *reinterpret_cast<DynamixelPacketHeader const *>(buffer + offset);
		uint16_t data_length = static_cast<uint16_t>(response_header.packet_length[0] + (response_header.packet_length[1] << 8) - 1);
		if (data_length > MAX_N_DATA_BYTES) {
			return false;
		}
		if (HAL_UART_Receive(&huart5, buffer, data_length, HAL_MAX_DELAY) != HAL_OK) {
			return false;
		}
		if (response_header.packet_id[0] != header.packet_id[0] || response_header.instruction[0] != INS_STATUS) {
			return false;
		}
		uint16_t crc = UpdateCRC(0, reinterpret_cast<uint8_t const *>(&response_header), sizeof(response_header));
		crc = UpdateCRC(crc, buffer, data_length - 2);
		if (crc != static_cast<uint16_t>(buffer[data_length-2] + (buffer[data_length-1] << 8))) {
			//if (header.packet_id[0] == 1) HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
			return false;
		}
		if (buffer[0]) {
			return false;
		}
		return true;
	}
}

DynamixelPacket::DynamixelPacket(uint8_t id, uint8_t ins, uint8_t const *parameters, size_t n_parameters)
	: header(id, n_parameters, ins)
{
	assert(id != 0xFD && id != 0xFF);
	assert(n_parameters <= MAX_N_PARAMETERS);
	assert(n_parameters == 0 || parameters);

	size_t bytes_of_forbidden_sequence = 0;
	size_t data_idx = 0;
	for (size_t i = 0; i < n_parameters; i++) {
		data[data_idx++] = parameters[i];
		if (parameters[i] == header.header[bytes_of_forbidden_sequence]) {
			++bytes_of_forbidden_sequence;
			if (bytes_of_forbidden_sequence >= sizeof(header.header) / sizeof(*header.header)) {
				data[data_idx++] = 0xFD;
				if (++header.packet_length[0] == 0) ++header.packet_length[1];
				bytes_of_forbidden_sequence = 0;
			}
		}
	}
	uint16_t crc = UpdateCRC(0, reinterpret_cast<uint8_t const *>(&header), sizeof(header));
	crc = UpdateCRC(crc, data, data_idx);
	data[data_idx++] = static_cast<uint8_t>(crc & 0xFF);
	data[data_idx++] = static_cast<uint8_t>((crc >> 8) & 0xFF);
	data_bytes = data_idx;
}

// http://support.robotis.com/en/product/actuator/dynamixel_pro/communication/crc.htm
uint16_t DynamixelPacket::UpdateCRC(uint16_t crc_accum, uint8_t const *data, size_t data_size) {
	constexpr uint16_t crc_table[256] = {
			0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
			0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
			0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
			0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
			0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
			0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
			0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
			0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
			0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
			0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
			0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
			0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
			0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
			0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
			0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
			0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
			0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
			0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
			0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
			0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
			0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
			0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
			0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
			0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
			0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
			0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
			0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
			0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
			0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
			0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
			0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
			0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202 };

	for (size_t j = 0; j < data_size; j++) {
		uint8_t i = (static_cast<uint8_t>(crc_accum >> 8) ^ data[j]) & 0xFF;
		crc_accum = static_cast<uint16_t>(crc_accum << 8) ^ crc_table[i];
	}

	return crc_accum;
}

size_t DynamixelPacket::BytesForAddress(Address address) {
	constexpr struct { Address addr; size_t bytes; } address_table[] = {
			{ADDR_ID, 1},
			{ADDR_TORQUE_ENABLE, 1},
			{ADDR_LED, 1},
			{ADDR_GOAL_POSITION, 4},
	};

	size_t bytes = 0;
	for (size_t i = 0; i < sizeof(address_table) / sizeof(*address_table); i++) {
		if (address_table[i].addr == address) {
			bytes = address_table[i].bytes;
			break;
		}
	}
	return bytes;
}

void SteeringServo::RegisterSetAngle(float radians) {
	if (servo_id == 0) return;

	long angle_count_64 = std::lround(radians * 4096.0f / (2.0f * M_PIf));
	int32_t angle_count = angle_count_64 > ANGLE_COUNT_MAX ? ANGLE_COUNT_MAX
			: angle_count_64 < -ANGLE_COUNT_MAX ? -ANGLE_COUNT_MAX
			: static_cast<int32_t>(angle_count_64);

	bool okay = DynamixelPacket::CreateWrite(
		servo_id, true, DynamixelPacket::ADDR_GOAL_POSITION, HOME_POSITION + static_cast<uint32_t>(angle_count)).SendToServo();
	if (!okay) {
		HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_SET);
	}
}

void SteeringServo::CommitSetAngles() {
	DynamixelPacket::CreateAction().SendToServo();
}

