using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

namespace Zippy {
    [RequireComponent(typeof(RawImage))]
    public abstract class CameraRigViewerBase : MonoBehaviour {
        protected CameraId _cameraId;
        protected RawImage _rawImage;
        protected bool _ready = false;
        private Material _material;

        void Start() {
            _rawImage = GetComponent<RawImage>();
            _rawImage.texture = null;
            _material = InitializeMaterial ();
            _rawImage.material = _material;
            ZippyEventManager.StartListening (ZippyEventType.CamerasReady, HandleCamerasReady);
            ZippyEventManager.StartListening (ZippyEventType.CameraEnabled, UpdateTexture);
        }

        private void OnDestroy() {
            ZippyEventManager.StopListening (ZippyEventType.CamerasReady, HandleCamerasReady);

            if (_material != null) {
                DestroyImmediate (_material);
                _rawImage.material = null;
                _material = null;
            }
        }

        void HandleCamerasReady(ZippyEventType zippyEvent, ZippyEventPayloadBase payload) {
            HandleCameraRigManagerInitialized ();
        }

        protected virtual void HandleCameraRigManagerInitialized() {
            _ready = true;
            SetRigTexture();
        }

        public CameraId SelectedCamera {
            get {
                return _cameraId;
            }
            set {
                _cameraId = value;

                if (_ready) {
                    SetRigTexture();
                }
            }
        }

        protected virtual Material InitializeMaterial() {
            return null;
        }

        protected abstract void SetRigTexture ();

        protected void SetRawImageUVRect(CameraRig rig) {
            if (rig.downloadedTextureOriginInTopLeftInsteadOfBottomLeft) {
                _rawImage.uvRect = new Rect(0, 1, 1, -1);
            }
            else {
                _rawImage.uvRect = new Rect(0, 0, 1, 1);
            }
        }

        protected void UpdateTexture(ZippyEventType zippyEvent, ZippyEventPayloadBase payload) {
            if (zippyEvent != ZippyEventType.CameraEnabled) {
                return;
            }

            var statusPayload = payload as CameraOutputPayload;

            if (statusPayload == null) {
                return;
            }

            if (SelectedCamera == statusPayload.cameraId) {
                SetRigTexture ();
            }
        }
    }
}