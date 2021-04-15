using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using UnityEngine;
using System.Runtime.InteropServices;

namespace Zippy {

    /// <summary>
    /// Ground truth vehicle pose publisher.
    /// </summary>
    public class UnityTelemetryPublisher : MonoBehaviour {

        public Transform torsoBottomCenter;

        /// <summary>
        /// Start this instance.
        /// </summary>
        private void Start() {
            //TODO define this high water mark value
            UnityTelemetryPublisherWrapper.initialize (ConnectionsReader.FullAddress(ConnectionTopic.ground_truth_telemetry), SimulatorSettingsManager.Settings.networking.zmqLingerTimeMs, 10);
        }

        void OnDestroy() {
            UnityTelemetryPublisherWrapper.stop ();
        }

        /// <summary>
        /// Frame-rate update.
        /// </summary>
        /// <returns>The update.</returns>
        private void Update() {
            UnityTelemetryPublisherWrapper.sendGroundTruthVehiclePose (torsoBottomCenter.position, torsoBottomCenter.rotation);
        }

    }

}
