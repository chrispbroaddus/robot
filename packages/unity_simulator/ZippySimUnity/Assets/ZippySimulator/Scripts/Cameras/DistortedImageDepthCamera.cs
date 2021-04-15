using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Zippy {
    /// <summary>
    /// Distorted camera.
    /// Ortothgraphic camera that views the distorted mesh and renders it to two textures, one for the image and one for the depth
    /// It is controlled and configured by the CameraRig script
    /// </summary>
    public class DistortedImageDepthCamera : DistortedCamera {
        //TODO do we get a speed up if we alternate render textures

        struct RTGroup {
            public RenderBuffer[] mrt;
            public RenderTexture imageRenderTexture;
            public RenderTexture depthRenderTexture;
        }

        RTGroup[] _rtGroup;
        int _activeGroup = 0;
        int _lastActive = 0;

        bool _isGreyscaleCamera = false;
        bool _depthImageEnabled = true;

        /// <summary>
        /// Configure the Distorted Camera. Set the cameraId, size and culling mask.
        /// </summary>
        /// <param name="cameraId">Camera identifier.</param>
        /// <param name="size">Size.</param>
        /// <param name="cullingMask">Culling mask.</param>
        public override void Configure(CameraId cameraId, Size size, int cullingMask) {
            var cameraSettings = SimulatorSettingsManager.CameraSettings (cameraId);
            _isGreyscaleCamera = cameraSettings.greyscale;
            string shaderName;
            _rtGroup = new RTGroup[SimulatorSettingsManager.Settings.cameras.frameDownloadBufferSize];

            if (_isGreyscaleCamera) {
                shaderName = "Zippy/Camera/MrtRDepth";
            }
            else {
                shaderName = "Zippy/Camera/MrtRGBDepth";
            }

            base.Configure (cameraId, size, cullingMask, shaderName);

            for (int ii = 0; ii < _rtGroup.Length; ii++) {
                _rtGroup [ii] = new RTGroup ();

                if (_isGreyscaleCamera) {
                    //FIXME This should be R8, but there are issues further down the chain when downloading the copied texture
                    _rtGroup [ii].imageRenderTexture = new RenderTexture(size.width, size.height, 24, RenderTextureFormat.ARGB32);
                }
                else {
                    _rtGroup [ii].imageRenderTexture = new RenderTexture(size.width, size.height, 24, RenderTextureFormat.ARGB32);
                }

                _rtGroup [ii].imageRenderTexture.filterMode = FilterMode.Bilinear;
                _rtGroup [ii].imageRenderTexture.wrapMode = TextureWrapMode.Clamp;
                _rtGroup [ii].imageRenderTexture.useMipMap = false;
                _rtGroup [ii].depthRenderTexture = new RenderTexture(size.width, size.height, 0, RenderTextureFormat.RFloat);
                _rtGroup [ii].depthRenderTexture.filterMode = FilterMode.Bilinear;
                _rtGroup [ii].depthRenderTexture.wrapMode = TextureWrapMode.Clamp;
                _rtGroup [ii].depthRenderTexture.useMipMap = false;
                _rtGroup [ii].mrt = new RenderBuffer[2];
                _rtGroup [ii].mrt[0] = _rtGroup [ii].imageRenderTexture.colorBuffer;
                _rtGroup [ii].mrt[1] = _rtGroup [ii].depthRenderTexture.colorBuffer;
            }

            _activeGroup = 0;
            _lastActive = _rtGroup.Length - 1;
        }

        void OnRenderImage(RenderTexture source, RenderTexture destination) {
            Graphics.SetRenderTarget(_rtGroup [_activeGroup].mrt, _rtGroup [_activeGroup].imageRenderTexture.depthBuffer);
            Graphics.Blit(source, _material);

            if (EnableCapture) {
                ImageType imageType = _isGreyscaleCamera ? ImageType.Greyscale : ImageType.Color;

                if (!StartDownload (_rtGroup [_activeGroup].imageRenderTexture, imageType)) {
                    Debug.LogError ("Failed to start downloading image texture");
                }

                if (_depthImageEnabled) {
                    if (!StartDownload (_rtGroup [_activeGroup].depthRenderTexture, ImageType.Depth)) {
                        Debug.LogError ("Failed to start downloading depth texture");
                    }
                }

                _lastActive = _activeGroup;
                _activeGroup++;

                if (_activeGroup >= _rtGroup.Length) {
                    _activeGroup = 0;
                }
            }
        }

        /// <summary>
        /// Gets the camera image.
        /// </summary>
        /// <value>The camera image.</value>
        public Texture CameraImage {
            get {
                return _rtGroup [_lastActive].imageRenderTexture;
            }
        }

        /// <summary>
        /// Gets the depth image.
        /// </summary>
        /// <value>The depth image.</value>
        public Texture DepthImage {
            get {
                return _rtGroup [_lastActive].depthRenderTexture;
            }
        }

        /// <summary>
        /// Set whether the depth images is enabled, and get its state
        /// </summary>
        /// <value><c>true</c> if depth image enabled; otherwise, <c>false</c>.</value>
        public bool DepthImageEnabled {
            get {
                return _depthImageEnabled;
            }
            set {
                _depthImageEnabled = value;
            }
        }
    }
}
