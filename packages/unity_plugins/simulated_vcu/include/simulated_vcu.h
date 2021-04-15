#pragma once

#include "packages/core/include/chrono.h"
#include "packages/hal/proto/vcu_command_envelope.pb.h"
#include "packages/unity_plugins/simulator_settings_reader/include/simulator_settings_reader.h"

#include <deque>
#include <mutex>
#include <thread>
#include <vector>

namespace unity_plugins {

class SimulatedVCU {

public:
    enum class ReceiverControlSignal { NONE = 0, KILL };

    enum class ConnectionStatus { NOT_CONNECTED = 0, CONNECTED };

    struct VehicleControlCommand {
        float frontLeftWheelTorque = 0;
        float frontLeftWheelSteerAngle = 0;

        float frontRightWheelTorque = 0;
        float frontRightWheelSteerAngle = 0;

        float middleLeftWheelTorque = 0;
        float middleLeftWheelSteerAngle = 0;

        float middleRightWheelTorque = 0;
        float middleRightWheelSteerAngle = 0;

        float rearLeftWheelTorque = 0;
        float rearLeftWheelSteerAngle = 0;

        float rearRightWheelTorque = 0;
        float rearRightWheelSteerAngle = 0;

        float delta_left_rail_servo = 0;
        float delta_right_rail_servo = 0;

        float delta_left_slider_value = 0;
        float delta_right_slider_value = 0;
    };

    struct WheelTelemetry {
        uint64_t gpsSystemtimeCount; // gps time in nanoseconds
        float centerBodyLinearVelocity; // linear velocity of the robot center
    };

    ///
    /// @brief Create a subscriber thread
    ///
    SimulatedVCU(const std::string& address, const float PIDKu, const float PIDTu, const VehicleCalibration& vehicleCalibration);

    ///
    /// @brief Destructor
    ///
    ~SimulatedVCU();

    ///
    /// @brief Copy constructor
    ///
    SimulatedVCU(const SimulatedVCU& obj) = delete;

    ///
    /// @brief Copy assignment constructor
    ///
    SimulatedVCU& operator=(SimulatedVCU other) = delete;

    ///
    /// @brief Update the wheel torques and angles based on the trajectory
    ///
    void update(const WheelTelemetry& wheelTelemetry);

    ///
    /// @brief Set the receiver control signal
    ///
    void setReceiverControlSignal(const ReceiverControlSignal& value);

    ///
    /// @brief Get the receiver control signal
    ///
    ReceiverControlSignal getReceiverControlSignal();

    ///
    /// @brief Set the connection status
    ///
    void setConnectionStatus(const ConnectionStatus& value);

    ///
    /// @brief Get the connection status
    ///
    ConnectionStatus getConnectionStatus();

    ///
    /// @brief Getter for the latest vehicle control command
    ///
    VehicleControlCommand getVehicleControlCommand() const;

private:
    ///
    /// @brief Receive a trajectory sample
    ///
    void receive(const std::string& address);

    ///
    /// @brief Calculate the steer angle from the input of inverseCurvature
    /// @detail Ref : https://zippy.quip.com/K48mABNgQbRB (ver 0.1, 5/5/2017)
    ///
    float getSteerAngle(const float targetWheelX, const float targetWheelY, const float inverseCurvature);

    ///
    /// @brief Read the latest trajectory command
    ///
    bool getTrajectoryCommand(hal::VCUTrajectoryCommand& trajectory, const std::chrono::nanoseconds& gpsSystemtime);

    ///
    /// @brief Read the trajectory command previous & closest to the gpsTime
    ///
    bool findTrajectorySegment(hal::VCUTrajectorySegment& trajectoryElement, const hal::VCUTrajectoryCommand& trajectory,
        const std::chrono::nanoseconds& gpsSystemtime);

    ///
    /// @brief Read the size of the trajectory commands
    ///
    int getTrajectoriesSize();

    ///
    /// @void Remove an outdated trajectory
    ///
    void removeOutdatedOldestTrajectory(const std::chrono::nanoseconds& currentTime);

    std::mutex m_mutexConnectionStatus;
    ConnectionStatus m_connectionStatus;

    std::thread m_thread;

    std::mutex m_mutexReceiverControlSignal;
    ReceiverControlSignal m_receiverControlSignal;

    std::mutex m_mutexTrajectories;
    std::deque<hal::VCUTrajectoryCommand> m_trajectoryCommandQueue;
    size_t m_maxNumTrajectories;

    VehicleControlCommand m_vehicleControlCommand;

    const VehicleCalibration m_vehicleCalibration;

    float m_integralVelocityError;

    std::chrono::nanoseconds m_prevGPSTime;

    float m_prevErrVelocity;

    // PID controller parameters
    const float m_PIDKu;
    const float m_PIDTu;
};
} // namespace unity_plugins
