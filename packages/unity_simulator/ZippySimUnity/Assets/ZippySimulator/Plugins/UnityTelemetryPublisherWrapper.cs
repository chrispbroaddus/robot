using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;
using UnityEngine;
using System.Runtime.InteropServices;

namespace Zippy {
    public static class UnityTelemetryPublisherWrapper {

        public static void initialize(string address, int zmqLingerTimeInMilliSeconds, int zmqHighWaterMarkValue) {
            UnityTelemetryPublisher_initialize (address, zmqLingerTimeInMilliSeconds, zmqHighWaterMarkValue);
        }

        public static void stop() {
            UnityTelemetryPublisher_stop ();
        }

        public static void sendGroundTruthVehiclePose(Vector3 pos, Quaternion quat) {
            Vector3 rod = Geometry.QuaternionToRodrigues (quat);
            UnityTelemetryPublisher_sendGroundTruthVehiclePose (pos.x, pos.y, pos.z, rod.x, rod.y, rod.z);
        }


        [DllImport("simulated_zippy", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern void UnityTelemetryPublisher_initialize(string address, int zmqLingerTimeInMilliSeconds, int zmqHighWaterMarkValue);

        [DllImport("simulated_zippy")]
        public static extern void UnityTelemetryPublisher_stop ();

        [DllImport("simulated_zippy")]
        public static extern void UnityTelemetryPublisher_sendGroundTruthVehiclePose(float x, float y, float z, float rodrigues_x, float rodrigues_y, float rodrigues_z);

    }

}
