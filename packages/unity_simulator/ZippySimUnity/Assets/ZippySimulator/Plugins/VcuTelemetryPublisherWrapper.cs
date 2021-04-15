#if (UNITY_EDITOR_LINUX || UNITY_STANDALONE_LINUX) && !(UNITY_EDITOR_OSX || UNITY_EDITOR_WIN)
#define IMAGE_SAVER_SUPPORTED
#endif

    using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;
using UnityEngine;
using System.Runtime.InteropServices;

namespace Zippy {
    // fixme: move to a separate file
    [StructLayout(LayoutKind.Sequential)]
    public struct WheelsRadius {
        public float leftFront;
        public float leftMiddle;
        public float leftRear;
        public float rightFront;
        public float rightMiddle;
        public float rightRear;
    }

    public enum VCUWheelID {
        LeftFrontWheel = 0,
        LeftMiddleWheel = 1,
        LeftRearWheel = 2,
        RightFrontWheel = 3,
        RightMiddleWheel = 4,
        RightRearWheel = 5,
    }

    public static class VcuTelemetryPublisherWrapper {

        public static void initialize(string address, int zmqLingerTimeInMilliSeconds, int zmqHighWaterMarkValue, WheelsRadius wheelsRadius) {
            VcuTelemetryPublisher_initialize (address, zmqLingerTimeInMilliSeconds, zmqHighWaterMarkValue, wheelsRadius);
        }

        public static void stop() {
            VcuTelemetryPublisher_stop ();
        }

        public static void sendWheelTelemetry(VCUWheelID wheelId, int timeDiffFromPrevInNanosecond, float distance) {
            VcuTelemetryPublisher_sendWheelTelemetry (wheelId, timeDiffFromPrevInNanosecond, distance);
        }

        public static void sendSliderTelemetry(VCUSliderID sliderId, float positionInMeters) {
            VcuTelemetryPublisher_sendSliderTelemetry (sliderId, positionInMeters);
        }

        public static void sendServoTelemetry(VCUServoID servoId, float angleInRadians) {
            VcuTelemetryPublisher_sendServoTelemetry (servoId, angleInRadians);
        }

        [DllImport("simulated_zippy", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern void VcuTelemetryPublisher_initialize(string address, int zmqLingerTimeInMilliSeconds, int zmqHighWaterMarkValue, WheelsRadius wheelsRadius);

        [DllImport("simulated_zippy")]
        public static extern void VcuTelemetryPublisher_stop ();

        [DllImport("simulated_zippy")]
        public static extern void VcuTelemetryPublisher_sendWheelTelemetry(VCUWheelID wheelId, int timeDiffFromPrevInNanosecond, float distance);

        [DllImport("simulated_zippy")]
        public static extern void VcuTelemetryPublisher_sendSliderTelemetry(VCUSliderID sliderId, float positionInMeters);

        [DllImport("simulated_zippy")]
        public static extern void VcuTelemetryPublisher_sendServoTelemetry(VCUServoID servoId, float angleInRadians);

    }

}
