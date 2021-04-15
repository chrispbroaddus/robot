#if (UNITY_EDITOR_LINUX || UNITY_STANDALONE_LINUX) && !(UNITY_EDITOR_OSX || UNITY_EDITOR_WIN)
#define IMU_STREAMER_SUPPORTED
#endif

    using UnityEngine;
using System.Runtime.InteropServices;
using System;

namespace Zippy {
    public static class IMUStreamerWrapper {
        public static bool IsPublisherRunning {
            get {
#if IMU_STREAMER_SUPPORTED
                return IMUStreamer_IsRunning() > 0;
#else
                return false;
#endif
            }
        }

        public static void StartPublisher() {
#if IMU_STREAMER_SUPPORTED

            if (IsPublisherRunning) {
                return;
            }

            IMUStreamer_Start (ConnectionsReader.FullAddress(ConnectionTopic.imu), SimulatorSettingsManager.Settings.networking.zmqLingerTimeMs, SimulatorSettingsManager.Settings.networking.zmqHighWaterMark);
#endif
        }

        public static void StopPublisher() {
#if IMU_STREAMER_SUPPORTED
            IMUStreamer_Stop ();
#endif
        }

        public static void SendReading(string name, System.UInt64 serialNumber, IMUReading reading) {
#if IMU_STREAMER_SUPPORTED
            MakeRightHanded(ref reading);
            IMUStreamer_SendReading(name, serialNumber, reading);
#endif
        }

        public static void SendReadings (string name, System.UInt64 serialNumber, IMUReading[] readings) {
            if (readings != null) {
                for (int ii = 0; ii < readings.Length; ii++) {
                    SendReading (name, serialNumber, readings [ii]);
                }
            }
        }

        static void MakeRightHanded(ref IMUReading reading) {
            //mapping left to right
            //x -> x, y -> z, z -> y
            //rotations negate
            float tmp = reading.accel.z;
            reading.accel.z = reading.accel.y;
            reading.accel.y = tmp;
            reading.gyro *= -1.0f;
            tmp = reading.gyro.z;
            reading.gyro.z = reading.gyro.y;
            reading.gyro.y = tmp;
        }

#if IMU_STREAMER_SUPPORTED
        [DllImport("simulated_zippy", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern void IMUStreamer_SendReading(string name, System.UInt64 serialNumber, IMUReading reading);

        [DllImport("simulated_zippy", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern void IMUStreamer_Start(string address, int zmqPubLingerTime, int zmqPubHighWaterMarkValue);

        [DllImport("simulated_zippy")]
        private static extern void IMUStreamer_Stop();

        [DllImport("simulated_zippy")]
        private static extern int IMUStreamer_IsRunning();
#endif
    }
}
