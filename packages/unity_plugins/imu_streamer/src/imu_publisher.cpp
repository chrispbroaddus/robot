#include "imu_publisher.h"
#include "packages/core/include/chrono.h"
#include "packages/core/proto/timestamp.pb.h"
#include "packages/hal/proto/imu_sample.pb.h"

#include <chrono>
#include <limits>

namespace {
constexpr auto IMU_TOPIC = "imu";
}

using namespace unity_plugins;

IMUPublisher::IMUPublisher(const std::string& address, const int zmqLingerTimeInMilliSeconds, const int zmqHighWaterMarkValue)
    : m_context(1)
    , m_publisher(m_context, address, zmqHighWaterMarkValue, zmqLingerTimeInMilliSeconds) {}

bool IMUPublisher::sendIMUReading(const std::string& name, uint64_t serialNumber, const IMUReading& imuReading) {

    // Note: System, gps or any real time time queries cannot be done here as this function is called as part of a non real time simulation
    // multiple times.
    // The unity simulation runs multiple updates with differnt times between rendered frames. The 'gps' time should be provided from unity

    auto device = new hal::Device();
    device->set_name(name);
    device->set_serialnumber(serialNumber);

    hal::IMUSample imuSample;
    imuSample.set_allocated_device(device);

    core::SystemTimestamp* systemTimestamp = new core::SystemTimestamp();
    systemTimestamp->set_nanos(imuReading.gpsSystemtimeCount);

    imuSample.set_allocated_systemtimestamp(systemTimestamp);

    auto hardwareTimestamp = new core::HardwareTimestamp();
    hardwareTimestamp->set_nanos(imuReading.gpsSystemtimeCount);

    imuSample.set_allocated_hardwaretimestamp(hardwareTimestamp);

    imuSample.add_gyro(imuReading.gyro.x);
    imuSample.add_gyro(imuReading.gyro.y);
    imuSample.add_gyro(imuReading.gyro.z);

    imuSample.add_accel(imuReading.accel.x);
    imuSample.add_accel(imuReading.accel.y);
    imuSample.add_accel(imuReading.accel.z);

    imuSample.add_mag(0);
    imuSample.add_mag(0);
    imuSample.add_mag(0);

    bool sendSuccess = m_publisher.send(imuSample, IMU_TOPIC);

    return sendSuccess;
}
