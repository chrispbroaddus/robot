using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

namespace Zippy {
    /// <summary>
    /// Camera rig depth viewer.
    /// Displays the depth image from a specified camera
    /// </summary>
    public class CameraRigDepthViewer : CameraRigViewerBase {
        public bool reshape = false;
        public bool invertDepthImage = false;

        CameraRig _rig;

        protected override Material InitializeMaterial() {
            var shader = Shader.Find("Zippy/UI/DepthViewer");
            return new Material(shader);
        }

        protected override void HandleCameraRigManagerInitialized() {
            CameraSettings settings = SimulatorSettingsManager.CameraSettings(_cameraId);
            float normMaxDepth = settings.maxDepthMapDistanceMeters / settings.farClippingPlaneDistanceMeters;
            _rawImage.material.SetFloat("_MaxDistance", normMaxDepth);
            _rawImage.material.SetFloat("_RenderingDepth", settings.farClippingPlaneDistanceMeters);
            _rawImage.material.SetFloat("_Invert", invertDepthImage ? 1 : 0);
            base.HandleCameraRigManagerInitialized ();
            ZippyEventManager.StartListening (ZippyEventType.CameraDepthEnabled, UpdateTexture);
        }

        void LateUpdate() {
            if (_rig == null || !_rig.CameraEnabled || !_rig.DepthImageEnabled) {
                _rawImage.texture = null;
            }
            else {
                _rawImage.texture = _rig.DistortedDepthImage;
            }
        }

        protected override void SetRigTexture() {
            _rig = CameraRigManager.Instance.GetCameraRig(_cameraId);

            if (_rig == null || !_rig.CameraEnabled || !_rig.DepthImageEnabled) {
                _rawImage.texture = null;
            }
            else {
                _rawImage.texture = _rig.DistortedDepthImage;

                if (reshape) {
                    float y = _rawImage.rectTransform.sizeDelta.y;
                    float x = _rawImage.rectTransform.sizeDelta.y * (float)_rawImage.texture.width / (float)_rawImage.texture.height;
                    _rawImage.rectTransform.sizeDelta = new Vector2 (x, y);
                }

                SetRawImageUVRect (_rig);
            }
        }
    }
}
