using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Zippy {
    [RequireComponent(typeof(Rigidbody))]
    public class IMUSimulator : MonoBehaviour {
        public string deviceName = "IMU Simulator";
        public uint deviceSerialNumber = 0;

        public bool autoStart = true;

        Rigidbody m_rigidbody;
        Vector3 m_lastGlobalFrameVelocity;
        Vector3 m_gravity;
        List<IMUReading> m_readings = new List<IMUReading>();
        bool m_imuActive = false;
        bool m_firstUpdate = false;
        ulong m_gpsTime = 0;

        // Use this for initialization
        void Start () {
            m_rigidbody = GetComponent<Rigidbody> ();
            m_gravity = Physics.gravity;
            IMUStreamerWrapper.StartPublisher ();
            Attach ();

            if (autoStart) {
                EnableIMU = true;
            }
        }

        void OnDestroy() {
            EnableIMU = false;
            IMUStreamerWrapper.StopPublisher ();
        }

        void Attach() {
            var parent = transform.parent;

            if (parent == null) {
                return;
            }

            var rb = parent.GetComponentInParent<Rigidbody> ();

            if (rb == null) {
                return;
            }

            var fixedJoint = GetComponent<FixedJoint> ();

            if (fixedJoint != null) {
                return;
            }

            fixedJoint = gameObject.AddComponent<FixedJoint> ();
            fixedJoint.connectedBody = rb;
            fixedJoint.breakForce = Mathf.Infinity;
            fixedJoint.breakTorque = Mathf.Infinity;
            m_rigidbody.useGravity = false;
        }

        public bool EnableIMU {
            get {
                return m_imuActive;
            }
            set {
                if (m_imuActive == value) {
                    return;
                }

                m_imuActive = value;

                if (!m_imuActive) {
                    m_readings.Clear ();
                    m_firstUpdate = true;
                }
            }
        }


        void FixedUpdate () {
            if (!m_imuActive) {
                return;
            }

            //on first update set the gps time. then on subsequent calls append the delta time.
            if (m_firstUpdate) {
                m_gpsTime = SimulatorTime.GPSSystemTimeCount;
                m_firstUpdate = false;
            }
            else {
                m_gpsTime = SimulatorTime.AddSeconds (m_gpsTime, Time.fixedDeltaTime);
            }

            var rotationInv = Quaternion.Inverse (m_rigidbody.rotation);
            Vector3 angularVelocity = rotationInv * m_rigidbody.angularVelocity;
            var globalFrameAcc = (m_rigidbody.velocity - m_lastGlobalFrameVelocity) / Time.deltaTime + m_gravity;
            Vector3 acceleration = rotationInv * globalFrameAcc;
            m_lastGlobalFrameVelocity = m_rigidbody.velocity;
            m_readings.Add(new IMUReading(acceleration, angularVelocity, m_gpsTime));
        }

        void Update() {
            if (!m_imuActive) {
                return;
            }

            if (m_readings.Count <= 0) {
                return;
            }

            var readings = m_readings.ToArray ();
            IMUStreamerWrapper.SendReadings(deviceName, deviceSerialNumber, readings);
            ZippyEventManager.SendEvent (ZippyEventType.IMUData, readings);
            m_readings.Clear ();
        }
    }
}
