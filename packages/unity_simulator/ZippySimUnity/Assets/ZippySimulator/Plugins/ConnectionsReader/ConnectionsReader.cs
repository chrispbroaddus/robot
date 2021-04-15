using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;
using System.Runtime.InteropServices;

namespace Zippy {
    public enum ConnectionTopic {
        front_left_stereo = 0,
        front_right_stereo = 1,
        rear_left_stereo = 2,
        rear_right_stereo = 3,
        front_fisheye = 4,
        rear_fisheye = 5,
        left_fisheye = 6,
        right_fisheye = 7,
        telemetry = 8,
        estimation = 9,
        execution = 10,
        docking = 11,
        station_finder = 12,
        wheel_odemetry = 13,
        trajectory = 14,
        gps = 15,
        imu = 16,
        fiducial_poses = 17,
        joystick = 18,
        ground_truth_telemetry = 19,
        simulator_commands = 20,
        simulator_stats = 21,
        front_left_stereo_depth = 22,
        front_right_stereo_depth = 23,
        rear_left_stereo_depth = 24,
        rear_right_stereo_depth = 25,
        front_fisheye_depth = 26,
        rear_fisheye_depth = 27,
        left_fisheye_depth = 28,
        right_fisheye_depth = 29,
        front_left_stereo_pointcloud = 30,
        front_right_stereo_pointcloud = 31,
        rear_left_stereo_pointcloud = 32,
        rear_right_stereo_pointcloud = 33,
        front_fisheye_pointcloud = 34,
        rear_fisheye_pointcloud = 35,
        left_fisheye_pointcloud = 36,
        right_fisheye_pointcloud = 37,
    };

    public static class ConnectionsReader {
        static readonly string DEFAULT_CONNECTIONS_FILENAME = "connections.json";
        static bool m_loaded = false;

        [DllImport("simulated_zippy", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern int ConnectionsReader_load (string filename);

        [DllImport("simulated_zippy")]
        private static extern int ConnectionsReader_hasConnection (ConnectionTopic topic);

        [DllImport("simulated_zippy", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern string ConnectionsReader_address(ConnectionTopic topic);

        [DllImport("simulated_zippy")]
        private static extern int ConnectionsReader_port (ConnectionTopic topic);

        [DllImport("simulated_zippy", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern string ConnectionsReader_fullAddress (ConnectionTopic topic);

        static void Load() {
            if (m_loaded) {
                return;
            }

            string filename = CommandLineParser.GetArgValue ("-connections");

            if (string.IsNullOrEmpty(filename)) {
                Debug.Log ("Using default connections file");
                filename = Path.Combine (Application.streamingAssetsPath, DEFAULT_CONNECTIONS_FILENAME);
            }

            int result = ConnectionsReader_load (filename);

            if (result == 0) {
                throw new System.InvalidOperationException ("Unable to load connections file: " + filename);
            }

            m_loaded = true;
        }

        public static bool HasConnection(ConnectionTopic topic) {
            Load ();
            return ConnectionsReader_hasConnection (topic) != 0;
        }

        public static string Address(ConnectionTopic topic) {
            Load ();
            return ConnectionsReader_address (topic);
        }

        public static int Port(ConnectionTopic topic) {
            Load ();
            return ConnectionsReader_port (topic);
        }

        public static string FullAddress(ConnectionTopic topic) {
            if (!ConnectionsReader.HasConnection(topic)) {
                throw new System.InvalidOperationException ("Connections file does not contain a connection for " + topic);
            }

            return ConnectionsReader_fullAddress(topic);
        }
    }

}
