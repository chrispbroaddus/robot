using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

namespace Zippy {

    /// <summary>
    /// Wheel controller.
    /// </summary>
    public class ZippyBotWheelsControllerUI : MonoBehaviour {
        public Rigidbody frontRightWheelRigidbody;
        public Rigidbody middleRightWheelRigidbody;
        public Rigidbody rearRightWheelRigidbody;
        public Rigidbody frontLeftWheelRigidbody;
        public Rigidbody middleLeftWheelRigidbody;
        public Rigidbody rearLeftWheelRigidbody;

        public Transform frontRightCylinder;
        public Transform rearRightCylinder;
        public Transform frontLeftCylinder;
        public Transform rearLeftCylinder;

        public Rigidbody centerRigidbody;

        public HingeJoint hingeJointTorsoVsLeftRail;
        public HingeJoint hingeJointTorsoVsRightRail;

        public float hingeJointHeightMin;
        public float hingeJointHeightMax;

        public float maxTargetVelocityMagnitude;
        public float maxSteeringAngle;

        private bool _skitSteering = false;

        private WheelController wheelController = new WheelController();

        bool m_keyboardControlEnabled = false;
        bool m_remoteJoystickControlEnabled = false;
        bool m_trajectoryControlEnabled = true;


        Dictionary<Transform, CachedTransform> _originalTransforms = new Dictionary<Transform, CachedTransform> ();
        Rigidbody[] _rigidBodies;
        bool _resetting = false;

        void Start() {
            var transforms = GetComponentsInChildren<Transform> (true);

            foreach (var t in transforms) {
                _originalTransforms [t] = new CachedTransform (t);
            }

            _rigidBodies = GetComponentsInChildren<Rigidbody> (true);
            wheelController.Start ();
            float PIDKu = 50f;
            float PIDTu = 0.6f;
            wheelController.InitializeTrajectoryCommandClient (ConnectionsReader.FullAddress(ConnectionTopic.trajectory), PIDKu, PIDTu);
            //set up joystick subscriber
            string joystickFullAddress = ConnectionsReader.FullAddress(ConnectionTopic.joystick);
            int timeoutInMilliseconds = SimulatorSettingsManager.Settings.networking.zmqSendReceiveTimeoutMs;
            int numJoystickAxis = 2;
            double maxLatencyInSec = 0.01;
            JoystickSubscriberWrapper.initialize (joystickFullAddress, timeoutInMilliseconds, numJoystickAxis, maxLatencyInSec);
            ZippyEventManager.StartListening (ZippyEventType.JoystickControl, HandleControlEvent);
            ZippyEventManager.StartListening (ZippyEventType.KeyboardControl, HandleControlEvent);
            ZippyEventManager.StartListening (ZippyEventType.TrajectoryControl, HandleControlEvent);
            ZippyEventManager.SendEvent (ZippyEventType.RobotReady);
        }

        void OnDestroy() {
            if (SimulatedVCUWrapper.isConnected()) {
                SimulatedVCUWrapper.stop();
            }

            ZippyEventManager.StopListening (ZippyEventType.JoystickControl, HandleControlEvent);
            ZippyEventManager.StopListening (ZippyEventType.KeyboardControl, HandleControlEvent);
            ZippyEventManager.StopListening (ZippyEventType.TrajectoryControl, HandleControlEvent);
        }

        void HandleControlEvent(ZippyEventType zippyEvent, ZippyEventPayloadBase payload) {
            var p = payload as ZippyEventPayload<bool>;

            if (p == null) {
                return;
            }

            switch (zippyEvent) {
            case ZippyEventType.JoystickControl:
                m_remoteJoystickControlEnabled = p.value;
                break;

            case ZippyEventType.TrajectoryControl:
                m_trajectoryControlEnabled = p.value;
                break;

            case ZippyEventType.KeyboardControl:
                m_keyboardControlEnabled = p.value;
                break;

            default:
                break;
            }
        }

        /// <summary>
        /// Update the torques applied on the wheels
        /// Must be done in FixedUpdate as is phyisics sim affecting rigidbodies
        /// </summary>
        private void FixedUpdate() {
            if (_resetting) {
                return;
            }

            if (m_trajectoryControlEnabled && SimulatedVCUWrapper.isConnected ()) {
                wheelController.UpdateFromTrajectoryCommand (
                    frontRightWheelRigidbody,
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
                    hingeJointTorsoVsLeftRail,
                    hingeJointTorsoVsRightRail,
                    hingeJointHeightMin,
                    hingeJointHeightMax,
                    _skitSteering);
            }
            else if (m_remoteJoystickControlEnabled) {
                wheelController.UpdateFromJoystickInput (frontRightWheelRigidbody,
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
                        maxSteeringAngle);
            }
            else if (m_keyboardControlEnabled) {
                wheelController.UpdateFromKeyboardInput (frontRightWheelRigidbody,
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
                        maxSteeringAngle);
            }

            SendStatusEvents ();
        }

        void SendStatusEvents() {
            var newVel = Quaternion.Inverse(centerRigidbody.rotation) * centerRigidbody.velocity;
            ZippyEventManager.SendEvent (ZippyEventType.Velocity, newVel);
        }

        public void Reset() {
            Debug.Log ("Resetting Zippy");
            _resetting = true;
            StartCoroutine (ResetRobot ());
        }

        IEnumerator ResetRobot() {
            foreach (var rb in _rigidBodies) {
                rb.velocity = Vector3.zero;
                rb.angularVelocity = Vector3.zero;
                rb.isKinematic = true;
                rb.detectCollisions = false;
                rb.Sleep ();
            }

            foreach (var kvp in _originalTransforms) {
                kvp.Value.Apply (kvp.Key);
            }

            yield return new WaitForFixedUpdate();
            yield return new WaitForFixedUpdate();

            foreach (var rb in _rigidBodies) {
                rb.isKinematic = false;
                rb.detectCollisions = true;
            }

            _resetting = false;
        }
    }
}

