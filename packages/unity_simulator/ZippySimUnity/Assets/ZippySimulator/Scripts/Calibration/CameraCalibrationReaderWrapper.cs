using System;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;
using UnityEngine;
using System.Runtime.InteropServices;
using System.IO;

namespace Zippy {
    public static class CameraCalibrationReaderWrapper {
        static readonly string DEFAULT_CALIBRATION_FILENAME = "system_calibration.json";

        // TODO: deprecate m_isInitialized with exception handling (SIM-54)
        private static bool m_isInitialized = false;

        /// <summary>
        /// Initialize this instance.
        /// </summary>
        private static void Initialize() {
            var filename = CommandLineParser.GetArgValue ("-calibration");

            if (string.IsNullOrEmpty(filename)) {
                Debug.Log ("Using default system calibration file");
                filename = Path.Combine (Application.streamingAssetsPath, DEFAULT_CALIBRATION_FILENAME);
            }

            m_isInitialized = (CameraCalibrationReader_Load (filename) == 1);

            if (!m_isInitialized) {
                throw new System.Exception ("Could not load system calibration file: " + filename);
            }
        }

        /// <summary>
        /// Read the specified calibrations and cameraId.
        /// </summary>
        /// <param name="calibrations">Calibrations.</param>
        /// <param name="cameraId">Camera identifier.</param>
        public static CameraCalibration Read(CameraId cameraId) {
            if (!m_isInitialized) {
                Initialize ();
            }

            var zoom = SimulatorSettingsManager.CameraSettings (cameraId).imageZoom;
            return new CameraCalibration(CameraCalibrationReader_Read (cameraId), zoom);
        }

        /// <summary>
        /// Read the extrinsic calibration from the specified sourceFrameId and targetFrameId.
        /// </summary>
        /// <param name="sourceFrameId">Source frame identifier.</param>
        /// <param name="targetFrameId">Target frame identifier.</param>
        public static ExtrinsicCalibrationToMarshal Read(CoordinateFrameId sourceFrameId, CoordinateFrameId targetFrameId) {
            if (!m_isInitialized) {
                Initialize ();
            }

            return ExtrinsicCalibrationReader_Read (sourceFrameId, targetFrameId);
        }

        [DllImport("simulated_zippy", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern int CameraCalibrationReader_Load (string filename);

        [DllImport("simulated_zippy")]
        private static extern KB4CameraCalibrationToMarshal CameraCalibrationReader_Read (CameraId cameraId);

        [DllImport("simulated_zippy")]
        private static extern ExtrinsicCalibrationToMarshal ExtrinsicCalibrationReader_Read (CoordinateFrameId sourceFrameId, CoordinateFrameId targetFrameId);
    }

}