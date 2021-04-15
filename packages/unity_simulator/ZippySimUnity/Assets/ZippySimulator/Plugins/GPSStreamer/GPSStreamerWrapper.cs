#if (UNITY_EDITOR_LINUX || UNITY_STANDALONE_LINUX) && !(UNITY_EDITOR_OSX || UNITY_EDITOR_WIN)
#define GPS_STREAMER_SUPPORTED
#endif

    using UnityEngine;
using System.Runtime.InteropServices;
using System;

namespace Zippy {
    public static class GPSStreamerWrapper {
        public static bool IsPublisherRunning {
            get {
#if GPS_STREAMER_SUPPORTED
                return GPSStreamer_IsRunning() > 0;
#else
                return false;
#endif
            }
        }

        public static void StartPublisher() {
#if GPS_STREAMER_SUPPORTED

            if (IsPublisherRunning) {
                return;
            }

            GPSStreamer_Start (ConnectionsReader.FullAddress(ConnectionTopic.gps), SimulatorSettingsManager.Settings.networking.zmqLingerTimeMs, SimulatorSettingsManager.Settings.networking.zmqHighWaterMark);
#endif
        }

        public static void StopPublisher() {
#if GPS_STREAMER_SUPPORTED
            GPSStreamer_Stop ();
#endif
        }

        public static void SendReading(GPSReading reading) {
#if GPS_STREAMER_SUPPORTED
            GPSStreamer_SendReading(reading);
#endif
        }

#if GPS_STREAMER_SUPPORTED
        [DllImport("simulated_zippy")]
        private static extern void GPSStreamer_SendReading(GPSReading reading);

        [DllImport("simulated_zippy", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern void GPSStreamer_Start(string address, int zmqPubLingerTime, int zmqPubHighWaterMarkValue);

        [DllImport("simulated_zippy")]
        private static extern void GPSStreamer_Stop();

        [DllImport("simulated_zippy")]
        private static extern int GPSStreamer_IsRunning();
#endif
    }
}
