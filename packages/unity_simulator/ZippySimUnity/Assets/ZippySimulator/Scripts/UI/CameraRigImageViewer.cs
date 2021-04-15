using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

namespace Zippy {
    /// <summary>
    /// Camera rig image viewer.
    /// Displays the image from a specified CameraRig
    /// </summary>
    [RequireComponent(typeof(RawImage))]
    public class CameraRigImageViewer : CameraRigViewerBase {
        public bool reshape = false;

        CameraRig _rig;

        protected override Material InitializeMaterial () {
            var shader = Shader.Find("Zippy/UI/ImageViewer");
            return new Material(shader);
        }

        void LateUpdate() {
            if (_rig == null || !_rig.CameraEnabled) {
                _rawImage.texture = null;
            }
            else {
                _rawImage.texture = _rig.DistortedCameraImage;
            }
        }

        protected override void SetRigTexture() {
            _rig = CameraRigManager.Instance.GetCameraRig(_cameraId);

            if (_rig == null || !_rig.CameraEnabled) {
                _rawImage.texture = null;
            }
            else {
                _rawImage.texture = _rig.DistortedCameraImage;

                if (reshape) {
                    float y = _rawImage.rectTransform.sizeDelta.y;
                    float x = _rawImage.rectTransform.sizeDelta.y * (float)_rawImage.texture.width / (float)_rawImage.texture.height;
                    _rawImage.rectTransform.sizeDelta = new Vector2 (x, y);
                }

                if (SimulatorSettingsManager.CameraSettings(_rig.cameraId).greyscale) {
                    _rawImage.material.EnableKeyword("IS_GREYSCALE");
                }
                else {
                    _rawImage.material.DisableKeyword("IS_GREYSCALE");
                }

                SetRawImageUVRect (_rig);
            }
        }
    }
}
