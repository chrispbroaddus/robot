using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;

namespace Zippy {
    /// <summary>
    /// GPS simulator.
    /// </summary>
    public class GPSSimulator : MonoBehaviour {
        public string latLongAltOriginString = "37.362953, -121.952671, 0";
        public Vector3 unityOrigin = Vector3.zero;
        public float sampleIntervalMs = 1000.0f;
        public bool autoStartGPS = true;

        GlobalPosition _refGlobalPosition;
        Vector3 _refUnityPosition;
        Transform _transform;
        bool _gpsActive = false;

        bool _refUnityPositionSet = false;
        // Use this for initialization
        void Start () {
            GPSStreamerWrapper.StartPublisher ();

            if (_refGlobalPosition == null) {
                _refGlobalPosition = new GlobalPosition (latLongAltOriginString);
            }

            if (!_refUnityPositionSet) {
                _refUnityPosition = unityOrigin;
            }

            _transform = transform;

            if (autoStartGPS) {
                EnableGPS = true;
            }
        }

        void OnDestroy() {
            EnableGPS = false;
            GPSStreamerWrapper.StopPublisher ();
        }

        public void SetLatLonAltOrigin(double lat, double lon, double alt) {
            _refGlobalPosition = new GlobalPosition (lat, lon, alt);
        }

        public void SetUnityOrigin(Vector3 origin) {
            _refUnityPosition = origin;
            _refUnityPositionSet = true;
        }

        public bool EnableGPS {
            get {
                return _gpsActive;
            }
            set {
                if (_gpsActive == value) {
                    return;
                }

                _gpsActive = value;

                if (_gpsActive) {
                    StartCoroutine (GPSUpdate ());
                }
                else {
                    StopCoroutine (GPSUpdate ());
                }
            }
        }

        IEnumerator GPSUpdate() {
            while (_gpsActive) {
                yield return new WaitForSecondsRealtime (sampleIntervalMs / 1000.0f);
                var gp = GlobalPositioning.GlobalPositionFromUnityPosition (_refGlobalPosition, _refUnityPosition, _transform.position);
                GPSStreamerWrapper.SendReading (gp.ConvertToGPSReading());
                ZippyEventManager.SendEvent (ZippyEventType.GPSData, gp);
            }
        }
    }
}
