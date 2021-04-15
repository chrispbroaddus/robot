using System.Collections;
using System.Collections.Generic;
using System.Xml;
using UnityEngine;

namespace Zippy {
    /// <summary>
    /// Camera calibration.
    /// </summary>
    public class CameraCalibration {
        /// <summary>
        /// Gets the camera model.
        /// </summary>
        /// <value>The model.</value>
        public CameraModel Model {
            get;
            private set;
        }

        /// <summary>
        /// Gets the translation.
        /// </summary>
        /// <value>The translation.</value>
        public Vector3 Translation {
            get;
            private set;
        }

        /// <summary>
        /// Gets the rotation.
        /// </summary>
        /// <value>The rotation.</value>
        public Quaternion Rotation {
            get;
            private set;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="Zippy.CameraCalibration"/> class from KB4CameraCalibrationToMarshal from c++ marshaling
        /// </summary>
        /// <param name="cameraNode">Camera node.</param>
        public CameraCalibration(KB4CameraCalibrationToMarshal cal, float zoom) {
            float[] parameters = new float[8] {cal.focalLengthX, cal.focalLengthY, cal.centerX, cal.centerY, cal.kb4_0, cal.kb4_1, cal.kb4_2, cal.kb4_3};
            Model = new KB4Camera (cal.resolutionX, cal.resolutionY, parameters, zoom);
        }
    }
}
