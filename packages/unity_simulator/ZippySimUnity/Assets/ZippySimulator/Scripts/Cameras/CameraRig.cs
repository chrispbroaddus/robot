using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;

namespace Zippy {
    /// <summary>
    /// Camera rig.
    /// Encapsulates and acts as an interface to all the components that are used to simulate the real world camera.
    /// </summary>
    [RequireComponent(typeof(Camera))]
    public class CameraRig : MonoBehaviour {

        public CameraId cameraId;               //Id to uniquely specify this camera
        public SingleUnityLayer renderLayer;    //Layer rendering will be done on

        [Tooltip("Make the downloaded texture origin in top left instead of bottom left. This will flip the images in the unity editor")]
        public bool downloadedTextureOriginInTopLeftInsteadOfBottomLeft = false;

        Camera _camera;
        Shader[] _shaders = new Shader[2];
        Material[] _materials = new Material[2];
        RenderTexture[] _renderTextures = new RenderTexture[2];
        RenderBuffer[] _xyzRenderBuffers;
        const int IMAGE_DEPTH = 0;
        const int IMAGE_DEPTH_XYZ = 1;

        DistortionGroup _imageDepthDistortionGroup;
        DistortionGroup _xyzDistortionGroup;

        bool _xyzImageEnabled = false;

        CameraSettings _cameraSettings;

        float _renderDepth = 0;
        bool _enableCapture = false;

        static readonly int MIN_CAMERA_IMAGE_HEIGHT = 480;

        /// <summary>
        /// Gets the calibration for the camera
        /// </summary>
        /// <value>The calibration.</value>
        public CameraCalibration Calibration {
            get;
            private set;
        }


        void Start () {
            _cameraSettings = SimulatorSettingsManager.CameraSettings(cameraId);

            if (!SetUpImageEffect()) {
                return;
            }

            //configure the camera
            _camera = GetComponent<Camera>();
            _camera.depthTextureMode = DepthTextureMode.Depth;
            _camera.nearClipPlane = _cameraSettings.nearClippingPlaneDistanceMeters;
            _camera.farClipPlane = _cameraSettings.farClippingPlaneDistanceMeters;
            _camera.cullingMask = _camera.cullingMask & ~renderLayer.Mask;
            _camera.AddCommandBuffer (CameraEvent.AfterEverything, TextureDownloaderWrapper.UpdateDownloadsCommandBuffer ());
            _camera.depth = _renderDepth;
            //read the config file
            ReadConfig();
            //create the distortion rendering group
            CreateImageDepthDistortionGroup();
            XYZImageEnabled = _cameraSettings.xyzEnabled;
            DepthImageEnabled = _cameraSettings.depthEnabled;
            CameraEnabled = _cameraSettings.enabled;
        }

        void OnDestroy() {
            for (int ii = 0; ii < _renderTextures.Length; ii++) {
                _renderTextures [ii] = null;

                if (_materials[ii] != null) {
                    DestroyImmediate(_materials[ii]);
                    _materials [ii] = null;
                }
            }
        }

        bool SetUpImageEffect() {
            // Disable if we don't support image effects
            if (!SystemInfo.supportsImageEffects) {
                Debug.LogError("System does not support image effects. CameraRig disabled");
                enabled = false;
                return false;
            }

            //set up image and depth shader
            if (!SetUpImageEffect ("Zippy/Camera/RGBDepth", IMAGE_DEPTH)) {
                return false;
            }

            //Set up XYZ pointcloud shader
            if (!SetUpImageEffect ("Zippy/Camera/RGBDepthXYZ", IMAGE_DEPTH_XYZ)) {
                return false;
            }

            return true;
        }

        bool SetUpImageEffect(string shaderName, int index) {
            //set up image and depth shader
            _shaders[index] = Shader.Find(shaderName);

            // Disable the image effect if the shader can't run on the users graphics card
            if (!_shaders[index] || !_shaders[index].isSupported) {
                Debug.LogError(shaderName + " shader missing or not supported");
                enabled = false;
                return false;
            }

            _materials[index] = new Material(_shaders[index]);

            if (_cameraSettings.greyscale) {
                _materials[index].EnableKeyword("IS_GREYSCALE");
            }

            _materials[index].hideFlags = HideFlags.HideAndDontSave;
            return true;
        }

        /// <summary>
        /// Sets the render depth of the camera
        /// </summary>
        /// <param name="depth">Depth.</param>
        public void SetRenderDepth(float depth) {
            _renderDepth = depth;

            if (_camera != null) {
                _camera.depth = depth;
            }

            if (_imageDepthDistortionGroup != null) {
                _imageDepthDistortionGroup.Camera.SetRenderDepth (_renderDepth + 0.1f);
            }

            if (_xyzDistortionGroup != null) {
                _xyzDistortionGroup.Camera.SetRenderDepth (_renderDepth + 0.1f);
            }
        }

        void ReadConfig() {
            Calibration = CameraCalibrationReaderWrapper.Read (cameraId);
            ConfigureCamera();
        }

        void ConfigureCamera() {
            _camera.projectionMatrix = Calibration.Model.CalibrationPlaneProjectionMatrix(_camera.nearClipPlane, _camera.farClipPlane);
            var size = Calibration.Model.CalibratedTextureSize(_cameraSettings.calibratedPlaneScaleFactor);

            if (_cameraSettings.greyscale) {
                _renderTextures[IMAGE_DEPTH] = new RenderTexture(size.width, size.height, 24, RenderTextureFormat.RGFloat);
            }
            else {
                _renderTextures[IMAGE_DEPTH] = new RenderTexture(size.width, size.height, 24, RenderTextureFormat.ARGBFloat);
            }

            _renderTextures[IMAGE_DEPTH].filterMode = FilterMode.Bilinear;
            _renderTextures[IMAGE_DEPTH].wrapMode = TextureWrapMode.Clamp;
            _renderTextures[IMAGE_DEPTH].useMipMap = false;
            _camera.targetTexture = _renderTextures[IMAGE_DEPTH];
        }

        void ConfigureXYZOutput() {
            int w = _renderTextures [IMAGE_DEPTH].width;
            int h = _renderTextures [IMAGE_DEPTH].height;
            _renderTextures[IMAGE_DEPTH_XYZ] = new RenderTexture(w, h, 24, RenderTextureFormat.ARGBFloat);
            _renderTextures[IMAGE_DEPTH_XYZ].filterMode = FilterMode.Bilinear;
            _renderTextures[IMAGE_DEPTH_XYZ].wrapMode = TextureWrapMode.Clamp;
            _renderTextures[IMAGE_DEPTH_XYZ].useMipMap = false;
            var projectionMatrix = GL.GetGPUProjectionMatrix (_camera.projectionMatrix, true);
            _materials[IMAGE_DEPTH_XYZ].SetMatrix ("projInv", projectionMatrix.inverse);
            _xyzRenderBuffers = new RenderBuffer[2];
            _xyzRenderBuffers [0] = _renderTextures [IMAGE_DEPTH].colorBuffer;
            _xyzRenderBuffers [1] = _renderTextures [IMAGE_DEPTH_XYZ].colorBuffer;
        }

        void CreateImageDepthDistortionGroup() {
            var settings = new DistortionGroup.Settings ();
            settings.calibration = Calibration;
            settings.cameraId = cameraId;
            settings.downloadedTextureOriginInTopLeftInsteadOfBottomLeft = downloadedTextureOriginInTopLeftInsteadOfBottomLeft;
            settings.renderDepth = _camera.depth + 0.1f;
            settings.renderLayer = renderLayer;
            settings.renderTexture = _renderTextures [IMAGE_DEPTH];
            settings.outputType = DistortionGroup.OutputType.ImageDepth;
            _imageDepthDistortionGroup = DistortionGroup.Create (settings);
            _imageDepthDistortionGroup.EnableCapture = _enableCapture;
            _imageDepthDistortionGroup.Camera.SetRenderDepth (_renderDepth + 0.1f);
        }

        void CreateXYZDistortionGroup() {
            if (_xyzDistortionGroup != null) {
                return;
            }

            var settings = new DistortionGroup.Settings ();
            settings.calibration = Calibration;
            settings.cameraId = cameraId;
            settings.downloadedTextureOriginInTopLeftInsteadOfBottomLeft = downloadedTextureOriginInTopLeftInsteadOfBottomLeft;
            settings.renderDepth = _camera.depth + 0.1f;
            settings.renderLayer = renderLayer;
            settings.renderTexture = _renderTextures [IMAGE_DEPTH_XYZ];
            settings.outputType = DistortionGroup.OutputType.XYZ;
            _xyzDistortionGroup = DistortionGroup.Create (settings);
            _xyzDistortionGroup.EnableCapture = _enableCapture;
            _xyzDistortionGroup.Camera.SetRenderDepth (_renderDepth + 0.1f);
        }

        void OnRenderImage(RenderTexture source, RenderTexture destination) {
            //image, depth and xyz
            if (_xyzImageEnabled) {
                Graphics.SetRenderTarget(_xyzRenderBuffers, _renderTextures[IMAGE_DEPTH].depthBuffer);
                Graphics.Blit(source, _materials[IMAGE_DEPTH_XYZ]);
            }
            //image and depth only
            else {
                Graphics.Blit (source, destination, _materials [IMAGE_DEPTH]);
            }
        }

        /// <summary>
        /// Get or set enabling capturing (image downloading)
        /// </summary>
        /// <value><c>true</c> if enable capture; otherwise, <c>false</c>.</value>
        public bool EnableCapture {
            get {
                return _enableCapture;
            }
            set {
                _enableCapture = value;

                if (_imageDepthDistortionGroup) {
                    _imageDepthDistortionGroup.EnableCapture = value;
                }

                if (_xyzDistortionGroup != null) {
                    _xyzDistortionGroup.EnableCapture = value;
                }
            }
        }

        public bool XYZImageEnabled {
            get {
                return _xyzImageEnabled;
            }
            set {
                _xyzImageEnabled = value;

                if (_xyzDistortionGroup == null) {
                    if (!_xyzImageEnabled) {
                        return;
                    }

                    ConfigureXYZOutput ();
                    CreateXYZDistortionGroup ();
                }

                _xyzDistortionGroup.gameObject.SetActive (_xyzImageEnabled);

                if (!_xyzImageEnabled) {
                    TextureDownloaderWrapper.CleanUp (cameraId, ImageType.XYZ);
                }

                ZippyEventManager.SendEvent (ZippyEventType.CameraXYZEnabled, new CameraOutputPayload(cameraId, _xyzImageEnabled));
            }
        }

        public Texture CalibratedPlaneImage {
            get {
                return _renderTextures[IMAGE_DEPTH];
            }
        }

        /// <summary>
        /// Gets the distorted camera image.
        /// </summary>
        /// <value>The distorted camera image.</value>
        public Texture DistortedCameraImage {
            get {
                var distortedCamera = _imageDepthDistortionGroup.Camera as DistortedImageDepthCamera;
                return distortedCamera.CameraImage;
            }
        }

        /// <summary>
        /// Gets the distored depth image.
        /// </summary>
        /// <value>The distored depth image.</value>
        public Texture DistortedDepthImage {
            get {
                var distortedCamera = _imageDepthDistortionGroup.Camera as DistortedImageDepthCamera;
                return distortedCamera.DepthImage;
            }
        }

        public Texture DistortedXYZImage {
            get {
                if (_xyzDistortionGroup == null) {
                    return null;
                }

                var distortedCamera = _xyzDistortionGroup.Camera as DistortedXYZCamera;
                return distortedCamera.XYZImage;
            }
        }

        /// <summary>
        /// Sets the output resolution of the camera rig as a fraction of the original camera image size
        /// </summary>
        /// <param name="fractionOfOriginalSize">Fraction of original size.</param>
        public void SetResolution(float fractionOfOriginalSize) {
            fractionOfOriginalSize = Mathf.Clamp01(fractionOfOriginalSize);
            int height = (int)(fractionOfOriginalSize * (float)Calibration.Model.OriginalImageSize.height);
            SetResolution(height);
        }

        /// <summary>
        /// Sets the output resolution of the camera rig using the specified image height
        /// </summary>
        /// <param name="height">Height.</param>
        public void SetResolution(int height) {
            if (Calibration.Model.ImageSize.height == height) {
                return;
            }

            height = Mathf.Clamp(height, MIN_CAMERA_IMAGE_HEIGHT, Calibration.Model.OriginalImageSize.height);
            float scale = (float)height / (float) Calibration.Model.OriginalImageSize.height;
            Calibration.Model.Scale(scale);
            var size = Calibration.Model.ImageSize;
            //update camera settings
            ConfigureCamera();
            _imageDepthDistortionGroup.SetResolution (size, _renderTextures [IMAGE_DEPTH]);

            if (_xyzDistortionGroup != null) {
                _xyzDistortionGroup.SetResolution (size, _renderTextures [IMAGE_DEPTH_XYZ]);
            }
        }

        /// <summary>
        /// Set the camera to be enabled or not. Get the current enabled state
        /// </summary>
        /// <value><c>true</c> if camera enabled; otherwise, <c>false</c>.</value>
        public bool CameraEnabled {
            get {
                return isActiveAndEnabled;
            }
            set {
                gameObject.SetActive(value);

                if (_imageDepthDistortionGroup != null) {
                    _imageDepthDistortionGroup.gameObject.SetActive (value);
                }

                if (_xyzDistortionGroup != null) {
                    if (value) {
                        _xyzDistortionGroup.gameObject.SetActive (_xyzImageEnabled);
                    }
                    else {
                        _xyzDistortionGroup.gameObject.SetActive (false);
                    }
                }

                if (!value) {
                    TextureDownloaderWrapper.CleanUp (cameraId);
                }

                ZippyEventManager.SendEvent (ZippyEventType.CameraEnabled, new CameraOutputPayload(cameraId, isActiveAndEnabled));
            }
        }

        /// <summary>
        /// Set the camera to be enabled or not. Get the current enabled state
        /// </summary>
        /// <value><c>true</c> if depth image enabled; otherwise, <c>false</c>.</value>
        public bool DepthImageEnabled {
            get {
                var distortedCamera = _imageDepthDistortionGroup.Camera as DistortedImageDepthCamera;
                return distortedCamera.DepthImageEnabled;
            }
            set {
                var distortedCamera = _imageDepthDistortionGroup.Camera as DistortedImageDepthCamera;
                distortedCamera.DepthImageEnabled = value;

                if (!value) {
                    TextureDownloaderWrapper.CleanUp (cameraId, ImageType.Depth);
                }

                ZippyEventManager.SendEvent (ZippyEventType.CameraXYZEnabled, new CameraOutputPayload(cameraId, value));
            }
        }
    }
}
