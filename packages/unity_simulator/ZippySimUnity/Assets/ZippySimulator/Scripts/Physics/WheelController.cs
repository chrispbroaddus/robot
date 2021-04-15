using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using UnityEngine;
using UnityEngine.UI;

namespace Zippy {

    /// <summary>
    /// Wheel controller.
    /// </summary>
    public class WheelController {
        float m_integralVelocityError = 0;
        float m_prevErrVelocity = 0;

        VehicleControlCommand m_vehicleControlCommand;

        bool m_firstTrajectoryUpdate = true;
        ulong m_gpsTime;

        /// <summary>
        /// Start this instance.
        /// </summary>
        public void Start() {
            m_integralVelocityError = 0;
            m_prevErrVelocity = 0;
        }

        /// <summary>
        /// Initializes the trajectory command client.
        /// </summary>
        /// <returns>The trajectory command client.</returns>
        /// <param name="portNumber">Port number.</param>
        /// <param name="PIDKu">PID ku.</param>
        /// <param name="PIDTu">PID tu.</param>
        public void InitializeTrajectoryCommandClient (string address, float PIDKu, float PIDTu) {
            SimulatedVCUWrapper.initialize(address, PIDKu, PIDTu, SimulatorSettingsManager.Settings.vehicle.vehicleCalibration);
        }

        /// <summary>
        /// Updates from trajectory command.
        /// </summary>
        /// <returns>The from trajectory command.</returns>
        /// <param name="frontRightWheelRigidbody">Front right wheel rigidbody.</param>
        /// <param name="middleRightWheelRigidbody">Middle right wheel rigidbody.</param>
        /// <param name="rearRightWheelRigidbody">Rear right wheel rigidbody.</param>
        /// <param name="frontLeftWheelRigidbody">Front left wheel rigidbody.</param>
        /// <param name="middleLeftWheelRigidbody">Middle left wheel rigidbody.</param>
        /// <param name="rearLeftWheelRigidbody">Rear left wheel rigidbody.</param>
        /// <param name="centerRigidbody">Center rigidbody.</param>
        /// <param name="frontRightCylinder">Front right cylinder.</param>
        /// <param name="rearRightCylinder">Rear right cylinder.</param>
        /// <param name="frontLeftCylinder">Front left cylinder.</param>
        /// <param name="rearLeftCylinder">Rear left cylinder.</param>
        /// <param name="skitSteering">Skit steering.</param>
        public void UpdateFromTrajectoryCommand(
            Rigidbody frontRightWheelRigidbody,
            Rigidbody middleRightWheelRigidbody,
            Rigidbody rearRightWheelRigidbody,
            Rigidbody frontLeftWheelRigidbody,
            Rigidbody middleLeftWheelRigidbody,
            Rigidbody rearLeftWheelRigidbody,
            Rigidbody centerRigidbody,
            Transform frontRightCylinder,
            Transform rearRightCylinder,
            Transform frontLeftCylinder,
            Transform rearLeftCylinder,
            HingeJoint hingeJointTorsoVsLeftRail,
            HingeJoint hingeJointTorsoVsRightRail,
            float hingeJointHeightMin,
            float hingeJointHeightMax,
            bool skitSteering) {
            //on first update set the gps time. then on subsequent calls append the delta time.
            if (m_firstTrajectoryUpdate) {
                m_gpsTime = SimulatorTime.GPSSystemTimeCount;
                m_firstTrajectoryUpdate = false;
            }
            else {
                m_gpsTime = SimulatorTime.AddSeconds (m_gpsTime, Time.fixedDeltaTime);
            }

            var velocityInVehicleFrame = Quaternion.Inverse(centerRigidbody.rotation) *
                                         centerRigidbody.velocity;
            var signVelocity = velocityInVehicleFrame[2] > 0 ? 1 : -1;
            WheelTelemetry wheelTelemetry;
            wheelTelemetry.gpsSystemtimeCount = m_gpsTime;
            wheelTelemetry.centerBodyLinearVelocity = signVelocity * velocityInVehicleFrame.magnitude;
            //TODO should these be combined?
            SimulatedVCUWrapper.setWheelTelemetry(wheelTelemetry);
            m_vehicleControlCommand = SimulatedVCUWrapper.getVehicleControlCommand();

            if (!skitSteering) {
                UpdateWheelForceAndAngle(frontRightWheelRigidbody, frontRightCylinder, centerRigidbody.rotation, m_vehicleControlCommand.frontRightWheelTorque, m_vehicleControlCommand.frontRightWheelSteerAngle);
                UpdateWheelForce(middleRightWheelRigidbody, frontRightCylinder.rotation, rearRightCylinder.rotation, m_vehicleControlCommand.middleRightWheelTorque);
                UpdateWheelForceAndAngle(rearRightWheelRigidbody, rearRightCylinder, centerRigidbody.rotation, m_vehicleControlCommand.rearRightWheelTorque, m_vehicleControlCommand.rearRightWheelSteerAngle);
                UpdateWheelForceAndAngle(frontLeftWheelRigidbody, frontLeftCylinder, centerRigidbody.rotation, m_vehicleControlCommand.frontLeftWheelTorque, m_vehicleControlCommand.frontLeftWheelSteerAngle);
                UpdateWheelForce(middleLeftWheelRigidbody, frontLeftCylinder.rotation, rearLeftCylinder.rotation, m_vehicleControlCommand.middleLeftWheelTorque);
                UpdateWheelForceAndAngle(rearLeftWheelRigidbody, rearLeftCylinder, centerRigidbody.rotation, m_vehicleControlCommand.rearLeftWheelTorque, m_vehicleControlCommand.rearLeftWheelSteerAngle);
            }
            else {
                UpdateWheelForceAndAngle(frontRightWheelRigidbody, frontRightCylinder, centerRigidbody.rotation, m_vehicleControlCommand.frontRightWheelTorque, 0);
                UpdateWheelForce(middleRightWheelRigidbody, frontRightCylinder.rotation, rearRightCylinder.rotation, m_vehicleControlCommand.middleRightWheelTorque);
                UpdateWheelForceAndAngle(rearRightWheelRigidbody, rearRightCylinder, centerRigidbody.rotation, m_vehicleControlCommand.rearRightWheelTorque, 0);
                UpdateWheelForceAndAngle(frontLeftWheelRigidbody, frontLeftCylinder, centerRigidbody.rotation, m_vehicleControlCommand.frontLeftWheelTorque, 0);
                UpdateWheelForce(middleLeftWheelRigidbody, frontLeftCylinder.rotation, rearLeftCylinder.rotation, m_vehicleControlCommand.middleLeftWheelTorque);
                UpdateWheelForceAndAngle(rearLeftWheelRigidbody, rearLeftCylinder, centerRigidbody.rotation, m_vehicleControlCommand.rearLeftWheelTorque, 0);
            }

            // Docking control
            {
                //                UnityEngine.Debug.Log ("IK");
                //                UnityEngine.Debug.Log (vehicleControlCommand.delta_left_slider_value);
                //                UnityEngine.Debug.Log (vehicleControlCommand.delta_right_slider_value);
                //                UnityEngine.Debug.Log (vehicleControlCommand.delta_left_rail_servo);
                float sliderControlAlpha = -0.005f;
                float torsoPitchControlAlpha = -0.01f;
                updateSliderHeight(sliderControlAlpha * m_vehicleControlCommand.delta_left_slider_value, hingeJointTorsoVsLeftRail, hingeJointHeightMin, hingeJointHeightMax);
                updateSliderHeight(sliderControlAlpha * m_vehicleControlCommand.delta_right_slider_value, hingeJointTorsoVsRightRail, hingeJointHeightMin, hingeJointHeightMax);
                applyDeltaAngleSliderVsRail(torsoPitchControlAlpha * m_vehicleControlCommand.delta_left_rail_servo * Mathf.Rad2Deg, hingeJointTorsoVsLeftRail, hingeJointTorsoVsRightRail);
            }
        }

        /// <summary>
        /// Updates from keyboard input.
        /// </summary>
        /// <returns>The from keyboard input.</returns>
        /// <param name="frontRightWheelRigidbody">Front right wheel rigidbody.</param>
        /// <param name="middleRightWheelRigidbody">Middle right wheel rigidbody.</param>
        /// <param name="rearRightWheelRigidbody">Rear right wheel rigidbody.</param>
        /// <param name="frontLeftWheelRigidbody">Front left wheel rigidbody.</param>
        /// <param name="middleLeftWheelRigidbody">Middle left wheel rigidbody.</param>
        /// <param name="rearLeftWheelRigidbody">Rear left wheel rigidbody.</param>
        /// <param name="centerRigidbody">Center rigidbody.</param>
        /// <param name="frontRightCylinder">Front right cylinder.</param>
        /// <param name="rearRightCylinder">Rear right cylinder.</param>
        /// <param name="frontLeftCylinder">Front left cylinder.</param>
        /// <param name="rearLeftCylinder">Rear left cylinder.</param>
        /// <param name="maxSteeringAngle">Max steering angle.</param>
        ///
        /// TODO : move algorithm parts into c++ native plugin.
        ///
        public void UpdateFromKeyboardInput(Rigidbody frontRightWheelRigidbody,
                                            Rigidbody middleRightWheelRigidbody,
                                            Rigidbody rearRightWheelRigidbody,
                                            Rigidbody frontLeftWheelRigidbody,
                                            Rigidbody middleLeftWheelRigidbody,
                                            Rigidbody rearLeftWheelRigidbody,
                                            Rigidbody centerRigidbody,
                                            Transform frontRightCylinder,
                                            Transform rearRightCylinder,
                                            Transform frontLeftCylinder,
                                            Transform rearLeftCylinder,
                                            float maxTargetVelocityMagnitude,
                                            float maxSteeringAngle) {
            float targetVelocityScalar = 0f;
            float targetAngle = 0;

            if (Input.GetAxis("Vertical") > 0.1) {
                targetVelocityScalar = maxTargetVelocityMagnitude;
            }
            else if (Input.GetAxis("Vertical") < -0.1) {
                targetVelocityScalar = -maxTargetVelocityMagnitude;
            }

            targetAngle = maxSteeringAngle * Input.GetAxis("Horizontal");
            CommonUpdate (frontRightWheelRigidbody,
                          middleRightWheelRigidbody,
                          rearRightWheelRigidbody,
                          frontLeftWheelRigidbody,
                          middleLeftWheelRigidbody,
                          rearLeftWheelRigidbody,
                          centerRigidbody,
                          frontRightCylinder,
                          rearRightCylinder,
                          frontLeftCylinder,
                          rearLeftCylinder,
                          maxTargetVelocityMagnitude,
                          maxSteeringAngle,
                          targetVelocityScalar,
                          targetAngle);
        }

        /// <summary>
        /// Updates the height of the slider.
        /// </summary>
        /// <returns>The slider height.</returns>
        /// <param name="delta">Delta.</param>
        /// <param name="joint">Joint.</param>
        /// <param name="hingeJointHeightMin">Hinge joint height minimum.</param>
        /// <param name="hingeJointHeightMax">Hinge joint height max.</param>
        private void updateSliderHeight(float delta, HingeJoint joint, float hingeJointHeightMin, float hingeJointHeightMax) {
            var anchor = joint.anchor;
            var newHeight = anchor.y + delta;
            newHeight = System.Math.Max (hingeJointHeightMin, newHeight);
            newHeight = System.Math.Min (hingeJointHeightMax, newHeight);
            anchor.Set (anchor.x, newHeight, anchor.z);
            joint.anchor = anchor;
        }

        /// <summary>
        /// Applies the delta angle slider vs rail.
        /// </summary>
        /// <returns>The delta angle slider vs rail.</returns>
        /// <param name="delta">Delta.</param>
        /// <param name="hingeJointTorsoVsLeftRail">Hinge joint torso vs left rail.</param>
        /// <param name="hingeJointTorsoVsRightRail">Hinge joint torso vs right rail.</param>
        private void applyDeltaAngleSliderVsRail(float delta, HingeJoint hingeJointTorsoVsLeftRail, HingeJoint hingeJointTorsoVsRightRail) {
            var leftSpring = hingeJointTorsoVsLeftRail.spring;
            leftSpring.targetPosition -= delta;
            hingeJointTorsoVsLeftRail.spring = leftSpring;
            var rightSpring = hingeJointTorsoVsRightRail.spring;
            rightSpring.targetPosition -= delta;
            hingeJointTorsoVsRightRail.spring = rightSpring;
        }

        public void UpdateFromJoystickInput(Rigidbody frontRightWheelRigidbody,
                                            Rigidbody middleRightWheelRigidbody,
                                            Rigidbody rearRightWheelRigidbody,
                                            Rigidbody frontLeftWheelRigidbody,
                                            Rigidbody middleLeftWheelRigidbody,
                                            Rigidbody rearLeftWheelRigidbody,
                                            Rigidbody centerRigidbody,
                                            Transform frontRightCylinder,
                                            Transform rearRightCylinder,
                                            Transform frontLeftCylinder,
                                            Transform rearLeftCylinder,
                                            float maxTargetVelocityMagnitude,
                                            float maxSteeringAngle) {
            float targetVelocityScalar = 0f;
            float targetAngle = 0;
            var response = JoystickSubscriberWrapper.receive ();

            if (response.success == 0) {
                return;
            }

            targetVelocityScalar = - response.axis0 * maxTargetVelocityMagnitude;
            targetAngle = maxSteeringAngle * response.axis1;
            CommonUpdate (frontRightWheelRigidbody,
                          middleRightWheelRigidbody,
                          rearRightWheelRigidbody,
                          frontLeftWheelRigidbody,
                          middleLeftWheelRigidbody,
                          rearLeftWheelRigidbody,
                          centerRigidbody,
                          frontRightCylinder,
                          rearRightCylinder,
                          frontLeftCylinder,
                          rearLeftCylinder,
                          maxTargetVelocityMagnitude,
                          maxSteeringAngle,
                          targetVelocityScalar,
                          targetAngle);
        }

        // Used by keyboard and joystick
        void CommonUpdate(Rigidbody frontRightWheelRigidbody,
                          Rigidbody middleRightWheelRigidbody,
                          Rigidbody rearRightWheelRigidbody,
                          Rigidbody frontLeftWheelRigidbody,
                          Rigidbody middleLeftWheelRigidbody,
                          Rigidbody rearLeftWheelRigidbody,
                          Rigidbody centerRigidbody,
                          Transform frontRightCylinder,
                          Transform rearRightCylinder,
                          Transform frontLeftCylinder,
                          Transform rearLeftCylinder,
                          float maxTargetVelocityMagnitude,
                          float maxSteeringAngle,
                          float targetVelocityScalar,
                          float steeringAngle) {
            var timeDiffInSec = Time.fixedDeltaTime;
            var velocityInVehicleFrame = Quaternion.Inverse(centerRigidbody.rotation) * centerRigidbody.velocity;
            var signVelocity = velocityInVehicleFrame[2] > 0 ? 1 : -1;
            var linearVelocity = signVelocity * velocityInVehicleFrame.magnitude;
            var Ku = 5f;
            var Tu = 0.300f; // 300 msec
            // PID Controller
            var Kp = 0.6f * Ku;
            var Ki = 2f / Tu;
            var Kd = 0.125f * Tu;
            var errVelocity = targetVelocityScalar - linearVelocity;
            m_integralVelocityError += errVelocity * timeDiffInSec;
            var diffVelocityError = 0f;

            if (timeDiffInSec > 0) {
                diffVelocityError = (errVelocity - m_prevErrVelocity) / timeDiffInSec;
            }

            float motorTorque = Kp * errVelocity + Ki * m_integralVelocityError + Kd * diffVelocityError;
            UpdateWheelForceAndAngle (frontRightWheelRigidbody, frontRightCylinder, centerRigidbody.rotation, motorTorque, steeringAngle);
            UpdateWheelForce (middleRightWheelRigidbody, frontRightCylinder.rotation, rearRightCylinder.rotation, motorTorque);
            UpdateWheelForceAndAngle (rearRightWheelRigidbody, rearRightCylinder, centerRigidbody.rotation, motorTorque, -steeringAngle);
            UpdateWheelForceAndAngle (frontLeftWheelRigidbody, frontLeftCylinder, centerRigidbody.rotation, motorTorque, steeringAngle);
            UpdateWheelForce (middleLeftWheelRigidbody, frontLeftCylinder.rotation, rearLeftCylinder.rotation, motorTorque);
            UpdateWheelForceAndAngle (rearLeftWheelRigidbody, rearLeftCylinder, centerRigidbody.rotation, motorTorque, -steeringAngle);
            m_prevErrVelocity = errVelocity;
        }

        /// <summary>
        /// Updates the wheel.
        /// </summary>
        /// <param name="axleInfo">Axle info.</param>
        /// <param name="motorTorque">Motor torque.</param>
        /// <param name="steeringAngle">Steering angle.</param>
        private void UpdateWheelForceAndAngle(Rigidbody rigidbody, Transform cylinder, Quaternion centerBodyRotation, float motorForce, float steeringAngle) {
            var quatBtwnCenterVsWheel = Quaternion.Inverse (centerBodyRotation) * cylinder.rotation;
            var anglesBtwnCenterVsWheel = quatBtwnCenterVsWheel.eulerAngles;
            var diffSteeringAngle = steeringAngle - anglesBtwnCenterVsWheel [1];
            cylinder.rotation *= Quaternion.Euler(0, diffSteeringAngle, 0);
            Vector3 force;
            force.x = 0;
            force.y = 0;
            force.z = motorForce;
            var forceInWorldCoord = cylinder.rotation * force;
            rigidbody.AddForce(forceInWorldCoord);
        }

        /// <summary>
        /// Updates the wheel.
        /// </summary>
        /// <param name="axleInfo">Axle info.</param>
        /// <param name="motorTorque">Motor torque.</param>
        /// <param name="steeringAngle">Steering angle.</param>
        private void UpdateWheelForce(Rigidbody rigidbody, Quaternion rotation1, Quaternion rotation2, float motorForce) {
            var rotationMiddle = Quaternion.Slerp (rotation1, rotation2, 0.5f);
            Vector3 force;
            force.x = 0;
            force.y = 0;
            force.z = motorForce;
            var forceInWorldCoord = rotationMiddle * force;
            rigidbody.AddForce(forceInWorldCoord);
        }
    }
}
