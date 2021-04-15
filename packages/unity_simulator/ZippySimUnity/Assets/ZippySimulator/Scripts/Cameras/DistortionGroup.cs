using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Zippy {
    public class DistortionGroup : MonoBehaviour {
        static Vector3 _distortionGroupLocation = new Vector3(0.0f, 0.0f, 0.0f);
        DistortedCamera _distortedCamera;
        DistortedMesh _distortedMesh;
        bool _downloadedTextureOriginInTopLeftInsteadOfBottomLeft = false;
        CameraCalibration _calibration;
        CameraId _cameraId;
        SingleUnityLayer _renderLayer;

        public enum OutputType { ImageDepth, XYZ };

        public class Settings {
            public CameraId cameraId;
            public SingleUnityLayer renderLayer;
            public float renderDepth;
            public CameraCalibration calibration;
            public bool downloadedTextureOriginInTopLeftInsteadOfBottomLeft;
            public RenderTexture renderTexture;

            public OutputType outputType;
        }

        public static DistortionGroup Create(Settings settings) {
            var root = GameObject.Find("Distortion Cameras");

            if (root == null) {
                root = new GameObject("Distortion Cameras");
                root.transform.position = new Vector3(0, 1000.0f, 0);
                root.isStatic = true;
            }

            var rootTransform = root.transform;
            //container
            var distortionGroupGO = new GameObject(settings.cameraId + " Distortion Group");
            var distortionGroup = distortionGroupGO.AddComponent<DistortionGroup> ();
            distortionGroup.CreateInternal (settings, rootTransform);
            //increment for the next group
            _distortionGroupLocation.x += 1.5f;
            return distortionGroup;
        }

        void CreateInternal(Settings settings, Transform rootTransform) {
            _calibration = settings.calibration;
            _renderLayer = settings.renderLayer;
            _cameraId = settings.cameraId;
            _downloadedTextureOriginInTopLeftInsteadOfBottomLeft = settings.downloadedTextureOriginInTopLeftInsteadOfBottomLeft;
            Size size = _calibration.Model.ImageSize;
            var groupT = transform;
            groupT.SetParent(rootTransform, false);
            groupT.localPosition = _distortionGroupLocation;
            gameObject.layer = settings.renderLayer.LayerIndex;
            gameObject.isStatic = true;
            //camera
            var camGo = new GameObject("Distortion Camera");
            camGo.transform.SetParent(groupT, false);
            camGo.layer = settings.renderLayer.LayerIndex;
            camGo.isStatic = true;

            switch (settings.outputType) {
            case OutputType.ImageDepth:
                _distortedCamera = camGo.AddComponent<DistortedImageDepthCamera> ();
                break;

            case OutputType.XYZ:
                _distortedCamera = camGo.AddComponent<DistortedXYZCamera> ();
                break;

            default:
                throw new System.NotImplementedException ("Invalid Output Type specified");
            }

            _distortedCamera.Configure(settings.cameraId, size, settings.renderLayer.Mask);
            _distortedCamera.SetRenderDepth(settings.renderDepth);
            //planar mesh with adjusted uv coordinates
            var goPlane = new GameObject("Distorted Plane");
            goPlane.transform.SetParent(groupT, false);
            goPlane.layer = settings.renderLayer.LayerIndex;
            goPlane.isStatic = true;
            _distortedMesh = goPlane.AddComponent<DistortedMesh>();
            int rows = Mathf.CeilToInt(SimulatorSettingsManager.CameraSettings(settings.cameraId).distortedMeshRowFineness * (float)size.height);
            int cols = Mathf.CeilToInt(SimulatorSettingsManager.CameraSettings(settings.cameraId).distortedMeshColFineness * (float)size.width);
            float aspect = (float)size.height / (float)size.width;
            _distortedMesh.Create(rows,
                                  cols,
                                  aspect,
                                  UVCallback,
                                  _calibration.Model.ValidPixelMask,
                                  ValidMaskUVCallback);
            _distortedMesh.Texture = settings.renderTexture;
        }

        Vector2 UVCallback(Vector2 uvPoint) {
            Vector2 pxPoint = new Vector2();
            pxPoint.x = uvPoint.x * (float)_calibration.Model.ImageSize.width;

            if (_downloadedTextureOriginInTopLeftInsteadOfBottomLeft) {
                pxPoint.y = uvPoint.y * (float)_calibration.Model.ImageSize.height;
            }
            else {
                pxPoint.y = (1.0f - uvPoint.y) * (float)_calibration.Model.ImageSize.height;
            }

            var calPoint = _calibration.Model.UnprojectImageToCalibratedPoint(pxPoint);
            var texPoint = _calibration.Model.ProjectCalibratedPointToTextureCoordinate(calPoint);
            return texPoint;
        }

        Vector2 ValidMaskUVCallback(Vector2 uvPoint) {
            if (!_downloadedTextureOriginInTopLeftInsteadOfBottomLeft) {
                uvPoint.y = 1.0f - uvPoint.y;
            }

            return uvPoint;
        }

        /// <summary>
        /// Get or set enabling capturing (image downloading)
        /// </summary>
        /// <value><c>true</c> if enable capture; otherwise, <c>false</c>.</value>
        public bool EnableCapture {
            get {
                return _distortedCamera.EnableCapture;
            }
            set {
                _distortedCamera.EnableCapture = value;
            }
        }

        public DistortedCamera Camera {
            get {
                return _distortedCamera;
            }
        }

        public void SetResolution(Size size, Texture meshTexture) {
            _distortedCamera.Configure (_cameraId, size, _renderLayer.Mask);
            int rows = Mathf.CeilToInt(SimulatorSettingsManager.CameraSettings(_cameraId).distortedMeshRowFineness * (float)size.height);
            int cols = Mathf.CeilToInt(SimulatorSettingsManager.CameraSettings(_cameraId).distortedMeshColFineness * (float)size.width);
            float aspect = (float)size.height / (float)size.width;
            _distortedMesh.Create(rows, cols, aspect, UVCallback, _calibration.Model.ValidPixelMask, ValidMaskUVCallback);
            _distortedMesh.Texture = meshTexture;
        }
    }
}
