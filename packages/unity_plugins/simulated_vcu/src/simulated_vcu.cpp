// simulated_vcu.cpp : Defines the entry point for the console application.
//

#define _USE_MATH_DEFINES

#include <cmath>

#include "glog/logging.h"

#include "packages/hal/proto/vcu_command_response.pb.h"
#include "packages/net/include/zmq_rep_server.h"
#include "packages/planning/vcu_trajectory_check.h"
#include "packages/unity_plugins/simulated_vcu/include/simulated_vcu.h"
#include "packages/unity_plugins/utils/include/zippy_interface.h"

using namespace unity_plugins;

static std::mutex s_mutexSimulatedVCU;
static std::unique_ptr<SimulatedVCU> s_simulatedVCU;

extern "C" {
ZIPPY_INTERFACE_EXPORT void SimulatedVCU_initialize(
    const char* address, const float PIDKu, const float PIDTu, VehicleCalibration vehicleCalibration) {
    std::lock_guard<std::mutex> guard(s_mutexSimulatedVCU);
    s_simulatedVCU.reset(new SimulatedVCU(std::string(address), PIDKu, PIDTu, vehicleCalibration));
}

ZIPPY_INTERFACE_EXPORT void SimulatedVCU_setWheelTelemetry(SimulatedVCU::WheelTelemetry wheelTelemetry) {
    std::lock_guard<std::mutex> guard(s_mutexSimulatedVCU);
    s_simulatedVCU->update(wheelTelemetry);
}

ZIPPY_INTERFACE_EXPORT SimulatedVCU::VehicleControlCommand SimulatedVCU_getVehicleControlCommand() {
    std::lock_guard<std::mutex> guard(s_mutexSimulatedVCU);
    return s_simulatedVCU->getVehicleControlCommand();
}

ZIPPY_INTERFACE_EXPORT int SimulatedVCU_isConnected() {
    std::lock_guard<std::mutex> guard(s_mutexSimulatedVCU);
    if (s_simulatedVCU) {
        return s_simulatedVCU->getConnectionStatus() == SimulatedVCU::ConnectionStatus::CONNECTED ? 1 : 0;
    } else {
        return 0;
    }
}

ZIPPY_INTERFACE_EXPORT void SimulatedVCU_stop() {
    std::lock_guard<std::mutex> guard(s_mutexSimulatedVCU);
    if (s_simulatedVCU) {
        s_simulatedVCU->setReceiverControlSignal(SimulatedVCU::ReceiverControlSignal::KILL);
        auto ptr = s_simulatedVCU.release();
        delete ptr;
    }
}
}

SimulatedVCU::SimulatedVCU(const std::string& address, const float PIDKu, const float PIDTu, const VehicleCalibration& vehicleCalibration)
    : m_connectionStatus(ConnectionStatus::NOT_CONNECTED)
    , m_thread(&SimulatedVCU::receive, this, address)
    , m_maxNumTrajectories(100)
    , m_vehicleCalibration(vehicleCalibration)
    , m_integralVelocityError(0)
    , m_prevGPSTime(0)
    , m_PIDKu(PIDKu)
    , m_PIDTu(PIDTu) {}

SimulatedVCU::~SimulatedVCU() {
    setReceiverControlSignal(SimulatedVCU::ReceiverControlSignal::KILL);
    m_thread.join();
}

void SimulatedVCU::receive(const std::string& address) {

    zmq::context_t context(1);
    net::ZMQProtobufRepServer<hal::VCUCommandEnvelope, hal::VCUCommandResponse> server(context, address, 1000, 1000);

    setConnectionStatus(ConnectionStatus::CONNECTED);

    int count = 0;

    while (1) {

        if (getReceiverControlSignal() == ReceiverControlSignal::KILL) {
            return;
        }

        hal::VCUCommandEnvelope envelope;
        bool successRecv = server.recv(envelope);

        if (successRecv) {

            int error_reason;
            planning::commandValidate(envelope, error_reason);

            hal::VCUCommandResponse commandResponse;
            auto gpsSystemtime = core::chrono::gps::wallClockInNanoseconds();
            commandResponse.mutable_timestamp()->set_nanos(gpsSystemtime.count());
            commandResponse.set_sequencenumber(count++);

            if (error_reason == 0) {
                const auto& trajectoryCommand = envelope.trajectorycommand();

                std::lock_guard<std::mutex> guard(m_mutexTrajectories);

                if (!m_trajectoryCommandQueue.empty()) {
                    LOG(WARNING) << "There is an existing trajectory command in execution.";
                }
                if (m_trajectoryCommandQueue.size() == m_maxNumTrajectories) {
                    m_trajectoryCommandQueue.pop_front();
                }
                m_trajectoryCommandQueue.emplace_back(trajectoryCommand);
                hal::VCUCommandDisposition disposition(hal::VCUCommandDisposition::CommandAccepted);
                commandResponse.set_disposition(disposition);
            } else {
                LOG(WARNING) << __PRETTY_FUNCTION__ << " ... error reason : " << error_reason;
                hal::VCUCommandDisposition disposition(hal::VCUCommandDisposition::CommandRejected);
                commandResponse.set_disposition(disposition);
            }

            server.send(commandResponse);
        }
    }
}

bool SimulatedVCU::findTrajectorySegment(hal::VCUTrajectorySegment& trajectoryElement, const hal::VCUTrajectoryCommand& trajectory,
    const std::chrono::nanoseconds& gpsSystemtime) {
    CHECK(trajectory.segments_size() >= 2);
    for (int i = 0, j = 1; j < trajectory.segments_size(); i++, j++) {
        uint64_t iTimeNano = trajectory.segments(i).targetstarttime().nanos();
        uint64_t jTimeNano = trajectory.segments(j).targetstarttime().nanos();
        uint64_t cTimeNano = gpsSystemtime.count();

        if (cTimeNano >= iTimeNano && cTimeNano <= jTimeNano) {
            trajectoryElement = trajectory.segments(i);
            return true;
        }
    }
    return false;
}

void SimulatedVCU::update(const WheelTelemetry& wheelTelemetry) {

    // Note: System, gps or any real time time queries cannot be done here as this function is called as part of a non real time simulation
    // multiple times.
    // The unity simulation runs multiple updates with differnt times between rendered frames. The 'gps' time should be provided from unity

    auto gpsSystemtime = std::chrono::nanoseconds(wheelTelemetry.gpsSystemtimeCount);

    if (m_prevGPSTime.count() == 0) {
        m_prevGPSTime = gpsSystemtime;
    }

    VLOG(2) << __PRETTY_FUNCTION__ << "wheelTelemetry.centerBodyLinearVelocity : " << wheelTelemetry.centerBodyLinearVelocity;
    VLOG(2) << __PRETTY_FUNCTION__ << "wheelTelemetry.gpsSystemtimeCount : " << wheelTelemetry.gpsSystemtimeCount;

    float tgInvCrvCntr = 0;
    float tgLnrVelCntr = 0;

    VLOG(2) << __PRETTY_FUNCTION__ << "SimulatedVCU::update(), timestamp : " << gpsSystemtime.count();

    hal::VCUTrajectoryCommand trajectoryCommand;

    if (getTrajectoryCommand(trajectoryCommand, gpsSystemtime)) {

        CHECK(m_vehicleCalibration.center.x == 0);
        CHECK(m_vehicleCalibration.center.y == 0);

        VLOG(2) << __PRETTY_FUNCTION__ << "trajectoryCommand.elements_size() : " << trajectoryCommand.segments_size();

        if (trajectoryCommand.segments_size() >= 2) {

            VLOG(2) << __PRETTY_FUNCTION__ << "parsing trajectoryElement.";
            hal::VCUTrajectorySegment trajectoryElement;

            if (findTrajectorySegment(trajectoryElement, trajectoryCommand, gpsSystemtime)) {
                tgInvCrvCntr = trajectoryElement.arcdrive().curvatureinversemeters();
                tgLnrVelCntr = trajectoryElement.arcdrive().linearvelocitymeterspersecond();
            }
        }
    }

    VLOG(2) << __PRETTY_FUNCTION__ << " ... targeting curvature : " << tgInvCrvCntr;
    VLOG(2) << __PRETTY_FUNCTION__ << " ... targeting linear velocity : " << tgLnrVelCntr;

    // Convert "target inverse curvature of the vehicle center" into steer angle per wheel
    m_vehicleControlCommand.frontLeftWheelSteerAngle
        = getSteerAngle(m_vehicleCalibration.frontLeft.x, m_vehicleCalibration.frontLeft.y, tgInvCrvCntr);
    m_vehicleControlCommand.frontRightWheelSteerAngle
        = getSteerAngle(m_vehicleCalibration.frontRight.x, m_vehicleCalibration.frontRight.y, tgInvCrvCntr);

    m_vehicleControlCommand.middleLeftWheelSteerAngle
        = getSteerAngle(m_vehicleCalibration.middleLeft.x, m_vehicleCalibration.middleLeft.y, tgInvCrvCntr);
    m_vehicleControlCommand.middleRightWheelSteerAngle
        = getSteerAngle(m_vehicleCalibration.middleRight.x, m_vehicleCalibration.middleRight.y, tgInvCrvCntr);

    m_vehicleControlCommand.rearLeftWheelSteerAngle
        = getSteerAngle(m_vehicleCalibration.rearLeft.x, m_vehicleCalibration.rearLeft.y, tgInvCrvCntr);
    m_vehicleControlCommand.rearRightWheelSteerAngle
        = getSteerAngle(m_vehicleCalibration.rearRight.x, m_vehicleCalibration.rearRight.y, tgInvCrvCntr);

    VLOG(2) << __PRETTY_FUNCTION__
            << "m_vehicleControlCommand.frontLeftWheelSteerAngle : " << m_vehicleControlCommand.frontLeftWheelSteerAngle;
    VLOG(2) << __PRETTY_FUNCTION__
            << "m_vehicleControlCommand.frontRightWheelSteerAngle : " << m_vehicleControlCommand.frontRightWheelSteerAngle;
    VLOG(2) << __PRETTY_FUNCTION__
            << "m_vehicleControlCommand.middleLeftWheelSteerAngle : " << m_vehicleControlCommand.middleLeftWheelSteerAngle;
    VLOG(2) << __PRETTY_FUNCTION__
            << "m_vehicleControlCommand.middleRightWheelSteerAngle : " << m_vehicleControlCommand.middleRightWheelSteerAngle;
    VLOG(2) << __PRETTY_FUNCTION__
            << "m_vehicleControlCommand.rearLeftWheelSteerAngle : " << m_vehicleControlCommand.rearLeftWheelSteerAngle;
    VLOG(2) << __PRETTY_FUNCTION__
            << "m_vehicleControlCommand.rearRightWheelSteerAngle : " << m_vehicleControlCommand.rearRightWheelSteerAngle;

    const float Kp = 0.6f * m_PIDKu;
    const float Ki = 2.f / m_PIDTu;
    const float Kd = 0.125f * m_PIDTu;

    float errVelocity = tgLnrVelCntr - wheelTelemetry.centerBodyLinearVelocity;

    auto deltaTime = gpsSystemtime - m_prevGPSTime;
    std::chrono::duration<float> deltaTimeFloatSec = deltaTime;
    float timeDiffInSec = deltaTimeFloatSec.count();

    m_integralVelocityError += errVelocity * timeDiffInSec;

    float diffVelocityError = 0.f;
    if (timeDiffInSec > 0) {
        diffVelocityError = (errVelocity - m_prevErrVelocity) / timeDiffInSec;
    }

    VLOG(2) << __PRETTY_FUNCTION__ << "Kp : " << Kp;
    VLOG(2) << __PRETTY_FUNCTION__ << "errVelocity : " << errVelocity;
    VLOG(2) << __PRETTY_FUNCTION__ << "Ki : " << Ki;
    VLOG(2) << __PRETTY_FUNCTION__ << "m_integralVelocityError : " << m_integralVelocityError;
    VLOG(2) << __PRETTY_FUNCTION__ << "Kd : " << Kd;
    VLOG(2) << __PRETTY_FUNCTION__ << "diffVelocityError : " << diffVelocityError;

    float motorTorque = Kp * errVelocity + Ki * m_integralVelocityError + Kd * diffVelocityError;

    VLOG(2) << __PRETTY_FUNCTION__ << "Motor torque : " << motorTorque;

    m_vehicleControlCommand.frontLeftWheelTorque = motorTorque;
    m_vehicleControlCommand.frontRightWheelTorque = motorTorque;

    m_vehicleControlCommand.middleLeftWheelTorque = motorTorque;
    m_vehicleControlCommand.middleRightWheelTorque = motorTorque;

    m_vehicleControlCommand.rearLeftWheelTorque = motorTorque;
    m_vehicleControlCommand.rearRightWheelTorque = motorTorque;

    m_prevGPSTime = gpsSystemtime;
    m_prevErrVelocity = errVelocity;

    removeOutdatedOldestTrajectory(gpsSystemtime);
}

void SimulatedVCU::removeOutdatedOldestTrajectory(const std::chrono::nanoseconds& currentTime) {
    std::lock_guard<std::mutex> guard(m_mutexTrajectories);
    if (m_trajectoryCommandQueue.empty()) {
        return;
    }

    const auto& lastElem = m_trajectoryCommandQueue[0].segments(m_trajectoryCommandQueue[0].segments_size() - 1);
    const uint64_t lastElemTime = lastElem.targetstarttime().nanos();
    const uint64_t currTime = currentTime.count();

    if (lastElemTime < currTime) {
        m_trajectoryCommandQueue.pop_front();
    }
}

float SimulatedVCU::getSteerAngle(float targetWheelX, float targetWheelY, float inverseCurvature) {
    return atan(targetWheelY * inverseCurvature / (1 - targetWheelX * inverseCurvature)) * 180.f / M_PI;
}

int SimulatedVCU::getTrajectoriesSize() {
    std::lock_guard<std::mutex> guard(m_mutexTrajectories);
    return m_trajectoryCommandQueue.size();
}

bool SimulatedVCU::getTrajectoryCommand(hal::VCUTrajectoryCommand& trajectory, const std::chrono::nanoseconds& gpsSystemtime) {
    std::lock_guard<std::mutex> guard(m_mutexTrajectories);

    const uint64_t gpsSystemtimeCount = gpsSystemtime.count();

    for (auto it = m_trajectoryCommandQueue.begin(); it != m_trajectoryCommandQueue.end(); it++) {
        if (it->segments(0).targetstarttime().nanos() < gpsSystemtimeCount
            && it->segments(it->segments_size() - 1).targetstarttime().nanos() >= gpsSystemtimeCount) {
            trajectory = *it;
            return true;
        }
    }

    return false;
}

void SimulatedVCU::setReceiverControlSignal(const ReceiverControlSignal& value) {
    std::lock_guard<std::mutex> guard(m_mutexReceiverControlSignal);
    m_receiverControlSignal = value;
}

SimulatedVCU::ReceiverControlSignal SimulatedVCU::getReceiverControlSignal() {
    std::lock_guard<std::mutex> guard(m_mutexReceiverControlSignal);
    return m_receiverControlSignal;
}

void SimulatedVCU::setConnectionStatus(const ConnectionStatus& value) {
    std::lock_guard<std::mutex> guard(m_mutexConnectionStatus);
    m_connectionStatus = value;
}

SimulatedVCU::ConnectionStatus SimulatedVCU::getConnectionStatus() {
    std::lock_guard<std::mutex> guard(m_mutexConnectionStatus);
    return m_connectionStatus;
}

SimulatedVCU::VehicleControlCommand SimulatedVCU::getVehicleControlCommand() const { return m_vehicleControlCommand; }
