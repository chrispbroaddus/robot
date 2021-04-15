using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using UnityEngine;
using System.Runtime.InteropServices;

namespace Zippy {

    /// <summary>
    /// Wheel telemetry publisher.
    /// </summary>
    public class VcuTelemetryPublisher : MonoBehaviour {

        public GameObject leftFrontWheel;
        public GameObject leftMiddleWheel;
        public GameObject leftRearWheel;

        public GameObject rightFrontWheel;
        public GameObject rightMiddleWheel;
        public GameObject rightRearWheel;

        public HingeJoint leftRailJoint;
        public HingeJoint rightRailJoint;

        public Rigidbody centerRigidbody;

        private SphereCollider leftFrontWheelCollider;
        private SphereCollider leftMiddleWheelCollider;
        private SphereCollider leftRearWheelCollider;

        private SphereCollider rightFrontWheelCollider;
        private SphereCollider rightMiddleWheelCollider;
        private SphereCollider rightRearWheelCollider;

        public Transform frontRightCylinder;
        public Transform rearRightCylinder;
        public Transform frontLeftCylinder;
        public Transform rearLeftCylinder;

        private Vector3 prevLeftFrontWheelPosition;
        private Vector3 prevLeftMiddleWheelPosition;
        private Vector3 prevLeftRearWheelPosition;

        private Vector3 prevRightFrontWheelPosition;
        private Vector3 prevRightMiddleWheelPosition;
        private Vector3 prevRightRearWheelPosition;

        private bool isFirstFixedupdate = true;

        /// <summary>
        /// Start this instance.
        /// </summary>
        private void Start() {
            leftFrontWheelCollider = leftFrontWheel.GetComponent (typeof(SphereCollider)) as SphereCollider;
            leftMiddleWheelCollider = leftMiddleWheel.GetComponent (typeof(SphereCollider)) as SphereCollider;
            leftRearWheelCollider = leftRearWheel.GetComponent (typeof(SphereCollider)) as SphereCollider;
            rightFrontWheelCollider = rightFrontWheel.GetComponent (typeof(SphereCollider)) as SphereCollider;
            rightMiddleWheelCollider = rightMiddleWheel.GetComponent (typeof(SphereCollider)) as SphereCollider;
            rightRearWheelCollider = rightRearWheel.GetComponent (typeof(SphereCollider)) as SphereCollider;
            WheelsRadius wheelsRadius = new WheelsRadius();
            wheelsRadius.leftFront = leftFrontWheel.transform.lossyScale[1];
            wheelsRadius.leftMiddle = leftMiddleWheel.transform.lossyScale[1];
            wheelsRadius.leftRear = leftRearWheel.transform.lossyScale[1];
            wheelsRadius.rightFront = rightFrontWheel.transform.lossyScale[1];
            wheelsRadius.rightMiddle = rightMiddleWheel.transform.lossyScale[1];
            wheelsRadius.rightRear = rightRearWheel.transform.lossyScale[1];
            //TODO define this high water mark value
            VcuTelemetryPublisherWrapper.initialize (ConnectionsReader.FullAddress(ConnectionTopic.telemetry), SimulatorSettingsManager.Settings.networking.zmqLingerTimeMs, 10, wheelsRadius);
        }

        void OnDestroy() {
            VcuTelemetryPublisherWrapper.stop ();
        }

        /// <summary>
        /// Fixed-time-interval update.
        /// </summary>
        /// <returns>The update.</returns>
        private void Update() {
            if (!isFirstFixedupdate) {
                //var rotVehicleWrtGlobal = Quaternion.Inverse (centerRigidbody.rotation);
                var unitZVector = new Vector3 (0, 0, 1);
                float signLeftFrontDistance = Vector3.Dot(leftFrontWheelCollider.attachedRigidbody.position - prevLeftFrontWheelPosition, centerRigidbody.rotation * unitZVector) > 0 ? 1f : -1f;
                float leftFrontDistance = signLeftFrontDistance * Vector3.Magnitude (leftFrontWheelCollider.attachedRigidbody.position - prevLeftFrontWheelPosition);
                float signLeftMiddleDistance = Vector3.Dot(leftMiddleWheelCollider.attachedRigidbody.position - prevLeftMiddleWheelPosition, centerRigidbody.rotation * unitZVector) > 0 ? 1f : -1f;
                float leftMiddleDistance = signLeftMiddleDistance * Vector3.Magnitude (leftMiddleWheelCollider.attachedRigidbody.position - prevLeftMiddleWheelPosition);
                float signLeftRearDistance = Vector3.Dot(leftRearWheelCollider.attachedRigidbody.position - prevLeftRearWheelPosition, centerRigidbody.rotation * unitZVector) > 0 ? 1f : -1f;
                float leftRearDistance = signLeftRearDistance * Vector3.Magnitude (leftRearWheelCollider.attachedRigidbody.position - prevLeftRearWheelPosition);
                float signRightFrontDistance = Vector3.Dot(rightFrontWheelCollider.attachedRigidbody.position - prevRightFrontWheelPosition, centerRigidbody.rotation * unitZVector) > 0 ? 1f : -1f;
                float rightFrontDistance = signRightFrontDistance * Vector3.Magnitude (rightFrontWheelCollider.attachedRigidbody.position - prevRightFrontWheelPosition);
                float signRightMiddleDistance = Vector3.Dot(rightMiddleWheelCollider.attachedRigidbody.position - prevRightMiddleWheelPosition, centerRigidbody.rotation * unitZVector) > 0 ? 1f : -1f;
                float rightMiddleDistance = signRightMiddleDistance * Vector3.Magnitude (rightMiddleWheelCollider.attachedRigidbody.position - prevRightMiddleWheelPosition);
                float signRightRearDistance = Vector3.Dot(rightRearWheelCollider.attachedRigidbody.position - prevRightRearWheelPosition, centerRigidbody.rotation * unitZVector) > 0 ? 1f : -1f;
                float rightRearDistance = signRightRearDistance * Vector3.Magnitude (rightRearWheelCollider.attachedRigidbody.position - prevRightRearWheelPosition);
                var timeDiffInNanos = (int)(Time.deltaTime * 1e9);
                VcuTelemetryPublisherWrapper.sendWheelTelemetry (VCUWheelID.LeftFrontWheel, timeDiffInNanos, leftFrontDistance);
                VcuTelemetryPublisherWrapper.sendWheelTelemetry (VCUWheelID.LeftMiddleWheel, timeDiffInNanos, leftMiddleDistance);
                VcuTelemetryPublisherWrapper.sendWheelTelemetry (VCUWheelID.LeftRearWheel, timeDiffInNanos, leftRearDistance);
                VcuTelemetryPublisherWrapper.sendWheelTelemetry (VCUWheelID.RightFrontWheel, timeDiffInNanos, rightFrontDistance);
                VcuTelemetryPublisherWrapper.sendWheelTelemetry (VCUWheelID.RightMiddleWheel, timeDiffInNanos, rightMiddleDistance);
                VcuTelemetryPublisherWrapper.sendWheelTelemetry (VCUWheelID.RightRearWheel, timeDiffInNanos, rightRearDistance);
                VcuTelemetryPublisherWrapper.sendServoTelemetry (VCUServoID.LeftFrontServo, getAngleVehicleWrtWheel(frontLeftCylinder, centerRigidbody.rotation) * Mathf.PI / 180f);
                VcuTelemetryPublisherWrapper.sendServoTelemetry (VCUServoID.LeftRearServo, getAngleVehicleWrtWheel(rearLeftCylinder, centerRigidbody.rotation) * Mathf.PI / 180f);
                VcuTelemetryPublisherWrapper.sendServoTelemetry (VCUServoID.RightFrontServo, getAngleVehicleWrtWheel(frontRightCylinder, centerRigidbody.rotation) * Mathf.PI / 180f);
                VcuTelemetryPublisherWrapper.sendServoTelemetry (VCUServoID.RightRearServo, getAngleVehicleWrtWheel(rearRightCylinder, centerRigidbody.rotation) * Mathf.PI / 180f);

                switch (SimulatorSettingsManager.Settings.vehicle.model) {
                case SimulatedVehicleModel.BoxCar:
                    break;

                case SimulatedVehicleModel.ZippyFromCAD:
                default:
                    VcuTelemetryPublisherWrapper.sendServoTelemetry (VCUServoID.LeftRailHingeJoint, leftRailJoint.spring.targetPosition * Mathf.PI / 180f);
                    VcuTelemetryPublisherWrapper.sendServoTelemetry (VCUServoID.RightRailHingeJoint, rightRailJoint.spring.targetPosition * Mathf.PI / 180f);
                    VcuTelemetryPublisherWrapper.sendSliderTelemetry (VCUSliderID.LeftRailSlider, leftRailJoint.transform.position.y);
                    VcuTelemetryPublisherWrapper.sendSliderTelemetry (VCUSliderID.RightRailSlider, rightRailJoint.transform.position.y);
                    break;
                }
            }

            prevLeftFrontWheelPosition = leftFrontWheelCollider.attachedRigidbody.position;
            prevLeftMiddleWheelPosition = leftMiddleWheelCollider.attachedRigidbody.position;
            prevLeftRearWheelPosition = leftRearWheelCollider.attachedRigidbody.position;
            prevRightFrontWheelPosition = rightFrontWheelCollider.attachedRigidbody.position;
            prevRightMiddleWheelPosition = rightMiddleWheelCollider.attachedRigidbody.position;
            prevRightRearWheelPosition = rightRearWheelCollider.attachedRigidbody.position;
            isFirstFixedupdate = false;
        }

        private float getAngleVehicleWrtWheel(Transform cylinder, Quaternion centerBodyRotation) {
            var quatBtwnCenterVsWheel = Quaternion.Inverse (centerBodyRotation) * cylinder.rotation;
            var anglesBtwnCenterVsWheel = quatBtwnCenterVsWheel.eulerAngles;
            float ret = anglesBtwnCenterVsWheel [1];

            if (ret > 180f) {
                ret -= 360f;
            }

            if (ret < -180f) {
                ret += 360f;
            }

            return ret;
        }

    }

}
