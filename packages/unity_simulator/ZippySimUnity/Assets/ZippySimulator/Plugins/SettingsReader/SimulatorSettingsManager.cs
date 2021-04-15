using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;
using System.Runtime.InteropServices;
using System.Linq;

namespace Zippy {
    public static class SimulatorSettingsManager {
        static bool m_loaded = false;
        static SimulatorSettings m_settings = new SimulatorSettings ();

        static readonly string DEFAULT_SETTINGS_FILENAME = "simulator_settings.json";

        static void Load() {
            if (m_loaded) {
                return;
            }

            string settingsFilename = CommandLineParser.GetArgValue ("-settings");

            if (string.IsNullOrEmpty(settingsFilename)) {
                Debug.Log ("Using default settings file");
                settingsFilename = Path.Combine (Application.streamingAssetsPath, DEFAULT_SETTINGS_FILENAME);
            }

            int result = SimulatorSettingsReader_Load (ref m_settings, settingsFilename);

            if (result == 0) {
                throw new System.InvalidOperationException ("Unable to load settings file: " + settingsFilename);
            }

            m_loaded = true;
        }

        public static SimulatorSettings Settings {
            get {
                Load ();
                return m_settings;
            }
        }

        public static CameraSettings CameraSettings(CameraId cameraId) {
            Load ();
            return m_settings.cameras.camera.FirstOrDefault (o => o.cameraId == cameraId);
        }

        [DllImport("simulated_zippy", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern int SimulatorSettingsReader_Load (ref SimulatorSettings settings, string filename);
    }
}
