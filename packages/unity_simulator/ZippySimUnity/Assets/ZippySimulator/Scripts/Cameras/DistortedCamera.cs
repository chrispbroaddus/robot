using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Zippy {
    [RequireComponent(typeof(Camera))]
    public abstract class DistortedCamera : MonoBehaviour {
        protected CameraId _cameraId;
        protected Camera _camera;
        protected Shader _shader;
        protected Material _material;
        protected Size _size;

        public abstract void Configure (CameraId cameraId, Size size, int cullingMask);

        protected virtual void Configure(CameraId cameraId, Size size, int cullingMask, string shaderName) {
            _cameraId = cameraId;
            _size = size;
            float aspect = (float)size.height / (float)size.width;

            if (_camera == null) {
                _camera = GetComponent<Camera>();
                _camera.orthographic = true;
                _camera.nearClipPlane = -1.0f;
                _camera.farClipPlane = 1.0f;
                _camera.cullingMask = cullingMask;
                _camera.clearFlags = CameraClearFlags.Color;
            }

            _camera.orthographicSize = 0.5f * aspect;
            _camera.aspect = 1.0f / aspect;

            if (!SetUpImageEffect(shaderName)) {
                throw new System.Exception("Unable to set up DistortedCamera");
            }
        }

        /// <summary>
        /// Sets the render depth.
        /// </summary>
        /// <param name="depth">Depth.</param>
        public void SetRenderDepth(float depth) {
            _camera.depth = depth;
        }

        bool SetUpImageEffect(string shaderName) {
            // Disable if we don't support image effects
            if (!SystemInfo.supportsImageEffects) {
                Debug.LogError("System does not support image effects. DistortedCamera disabled");
                enabled = false;
                return false;
            }

            _shader = Shader.Find(shaderName);

            // Disable the image effect if the shader can't run on the users graphics card
            if (!_shader || !_shader.isSupported) {
                Debug.LogError(shaderName + " shader missing or not supported");
                enabled = false;
                return false;
            }

            _material = new Material(_shader);
            _material.hideFlags = HideFlags.HideAndDontSave;
            return true;
        }

        /// <summary>
        /// Enable capture (image downloading), and get its current state
        /// </summary>
        /// <value><c>true</c> if enable capture; otherwise, <c>false</c>.</value>
        public bool EnableCapture {
            get;
            set;
        }

        protected bool StartDownload(Texture tex, ImageType imageType) {
            var frameData = new FrameData(_cameraId, imageType, tex, Time.frameCount, Time.unscaledTime);
            var status = TextureDownloaderWrapper.StartDownload(frameData);

            if (TextureDownloaderWrapper.Failed(status)) {
                Debug.LogError(_cameraId + " failed to start downloading " + imageType + "texture: " + status);
                return false;
            }
            else {
                return true;
            }
        }
    }
}