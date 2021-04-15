/*
 * servo.h
 *
 *  Created on: May 24, 2017
 *      Author: addaon
 */

#ifndef SERVO_H_
#define SERVO_H_

#include <cstdint>
#include <cstddef>

// http://support.robotis.com/en/product/actuator/dynamixel_pro/communication/instruction_status_packet.htm
class DynamixelPacket {
public:
	enum Instruction : uint8_t {
		INS_PING = 0x01,
		INS_READ = 0x02,
		INS_WRITE = 0x03,
		INS_REG_WRITE = 0x04,
		INS_ACTION = 0x05,
		INS_FACTORY_RESET = 0x06,
		INS_REBOOT = 0x08,
		INS_STATUS = 0x55,
		INS_SYNC_READ = 0x82,
		INS_SYNC_WRITE = 0x83,
		INS_BULK_READ = 0x92,
		INS_BULK_WRITE = 0x93,
	};

	enum Address : uint16_t {
		ADDR_ID = 7,
		ADDR_TORQUE_ENABLE = 64,
		ADDR_LED = 65,
		ADDR_GOAL_POSITION = 116,
	};

	static DynamixelPacket CreateWrite(uint8_t id, bool registered, Address address, uint32_t value);
	static DynamixelPacket CreateFactoryReset(uint8_t id, uint8_t param);
	static DynamixelPacket CreateAction(uint8_t id = 0xFE);
	bool SendToServo();

	~DynamixelPacket() = default;
	DynamixelPacket(DynamixelPacket const &) = default;
	DynamixelPacket &operator =(DynamixelPacket const &) = default;

private:

	DynamixelPacket(uint8_t id, uint8_t ins, uint8_t const *parameters, size_t n_parameters);

	// http://support.robotis.com/en/product/actuator/dynamixel_pro/communication/crc.htm
	static uint16_t UpdateCRC(uint16_t crc_accum, uint8_t const *data, size_t data_size);
	static size_t BytesForAddress(Address address);

	constexpr static size_t MAX_N_PARAMETERS = 8;
	constexpr static size_t MAX_N_DATA_BYTES = (MAX_N_PARAMETERS + 1)  // +1 for return packet field
						   * 4 / 3		   // *4/3 for byte stuffing
						   + 2;			   // +2 for CRC

	struct DynamixelPacketHeader {
		DynamixelPacketHeader(uint8_t id, size_t n_parameters, uint8_t ins)
			: packet_id{id},
			  packet_length{static_cast<uint8_t>((n_parameters + 3) & 0xFF),
					static_cast<uint8_t>(((n_parameters + 3) >> 8) & 0xFF)},
			  instruction{ins}
		{}

		~DynamixelPacketHeader() = default;
		DynamixelPacketHeader(DynamixelPacketHeader const &) = default;
		DynamixelPacketHeader &operator =(DynamixelPacketHeader const &) = default;

		uint8_t const header[3] = {0xFF, 0xFF, 0xFD};
		uint8_t const reserved[1] = {0x00};
		uint8_t const packet_id[1];
		uint8_t packet_length[2];  // updated after construction due to byte stuffing
		uint8_t const instruction[1];
	} __attribute__((packed)) header;

	uint8_t data[MAX_N_DATA_BYTES];
	size_t data_bytes;

} __attribute__((packed));

class SteeringServo {
public:
	SteeringServo(uint8_t id) : servo_id{id} {}
	~SteeringServo() = default;
	SteeringServo(SteeringServo const &) = delete;
	SteeringServo &operator =(SteeringServo const &) = delete;

	void RegisterSetAngle(float radians);
	static void CommitSetAngles();

	constexpr static int32_t HOME_POSITION = 2048;  // center of range

private:

	uint8_t servo_id;

	constexpr static int32_t ANGLE_COUNT_MAX = 1023;  // quarter turn in either direction
};

#endif /* SERVO_H_ */
