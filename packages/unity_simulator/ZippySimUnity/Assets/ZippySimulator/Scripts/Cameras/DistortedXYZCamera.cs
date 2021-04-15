using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Zippy {
    public class DistortedXYZCamera : DistortedCamera {
        //TODO try having 2 alternating textures
        RenderTexture[] _xyzRenderTexture;
        int _activeTexture = 0;
        int _lastActive = 0;

        /// <summary>
        /// Configure the Distorted Camera. Set the cameraId, size and culling mask.
        /// </summary>
        /// <param name="cameraId">Camera identifier.</param>
        /// <param name="size">Size.</param>
        /// <param name="cullingMask">Culling mask.</param>
        public override void Configure(CameraId cameraId, Size size, int cullingMask) {
            base.Configure (cameraId, size, cullingMask, "Zippy/Camera/XYZ");
            _xyzRenderTexture = new RenderTexture[SimulatorSettingsManager.Settings.cameras.frameDownloadBufferSize];

            for (int ii = 0; ii < _xyzRenderTexture.Length; ii++) {
                _xyzRenderTexture[ii] = new RenderTexture(size.width, size.height, 0, RenderTextureFormat.ARGBFloat);
                _xyzRenderTexture[ii].filterMode = FilterMode.Bilinear;
                _xyzRenderTexture[ii].wrapMode = TextureWrapMode.Clamp;
                _xyzRenderTexture[ii].useMipMap = false;
                _xyzRenderTexture[ii].Create ();
            }

            _activeTexture = 0;
            _lastActive = _xyzRenderTexture.Length - 1;
            _camera.targetTexture = _xyzRenderTexture[_activeTexture];
        }

        void OnRenderImage(RenderTexture source, RenderTexture destination) {
            Graphics.Blit(source, destination, _material);

            if (EnableCapture) {
                if (!StartDownload (_xyzRenderTexture[_activeTexture], ImageType.XYZ)) {
                    Debug.LogError ("Unable to download XYZ texture");
                }

                _lastActive = _activeTexture;
                _activeTexture++;

                if (_activeTexture >= _xyzRenderTexture.Length) {
                    _activeTexture = 0;
                }

                _camera.targetTexture = _xyzRenderTexture[_activeTexture];
            }
        }

        /// <summary>
        /// Gets the camera image.
        /// </summary>
        /// <value>The camera image.</value>
        public Texture XYZImage {
            get {
                return _xyzRenderTexture[_lastActive];
            }
        }
    }
}
