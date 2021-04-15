using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

namespace Zippy {
    /// <summary>
    /// Camera rig calibration plane viewer.
    /// Displays the calibration plane image from a specified CameraRig
    /// </summary>
    [RequireComponent(typeof(RawImage))]
    public class CalibratedPlaneViewer : CameraRigViewerBase {
        protected override void SetRigTexture() {
            var rig = CameraRigManager.Instance.GetCameraRig(_cameraId);

            if (rig == null || !rig.CameraEnabled) {
                _rawImage.texture = null;
            }
            else {
                _rawImage.texture = rig.CalibratedPlaneImage;
            }
        }
    }
}
