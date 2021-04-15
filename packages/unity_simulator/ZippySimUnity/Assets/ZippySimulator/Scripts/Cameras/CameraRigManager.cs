using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using UnityEngine;
using UnityEngine.Rendering;

namespace Zippy {
    /// <summary>
    /// Camera rig manager.
    /// This is the main interface to control the cameras on Zippy.
    /// It is a singleton, for easy access.
    /// It can be used to enable and disable cameras and image modes,
    /// set camera resolution, set download complete callback
    /// </summary>
    public class CameraRigManager : SingletonBase<CameraRigManager> {
        /// <summary>
        /// Occurs when initialized. Connect to this to be notified when CameraRigManager is initialized
        /// </summary>
        public event System.Action OnInitialized;

        /// <summary>
        /// Set the offscreen render layer.
        /// </summary>
        public SingleUnityLayer offscreenRenderLayer;

        bool _initialized = false;      // initialized flag
        bool _captureFrames = false;    // Capture frame flag

        Dictionary<CameraId, CameraRig> _cameraRigs = new Dictionary<CameraId, CameraRig>();    //known rigs

        protected override void Awake() {
            TextureDownloaderWrapper.InitializeTextureDownloader ();
            base.Awake ();
        }

        // Use this for initialization
        IEnumerator Start() {
            //wait one frame for other scripts to start up
            yield return null;

            if (!_initialized) {
                Init();
            }

            yield return null;
            ZippyEventManager.SendEvent (ZippyEventType.CamerasReady);
            ZippyEventManager.SendEvent (ZippyEventType.EnabledCameras, EnabledCameras);
        }

        //Initialize the rig manager and the camera rigs
        void Init() {
            var rigs = GetComponentsInChildren<CameraRig>(true);
            float currentCameraRenderDepth = SimulatorSettingsManager.Settings.cameras.cameraStartDepth;

            foreach (var rig in rigs) {
                var cameraSettings = SimulatorSettingsManager.CameraSettings(rig.cameraId);
                //make sure they all render in a sensible order
                rig.SetRenderDepth(currentCameraRenderDepth++);
                //turn cameras on/off as required
                rig.CameraEnabled = cameraSettings.enabled;
                //add to dictionary of cameras
                _cameraRigs[rig.cameraId] = rig;
            }

            //start the download update coroutine
            StartCoroutine(UpdateDownloads());
            _initialized = true;

            // send the initialized event
            if (OnInitialized != null) {
                OnInitialized();
            }
        }

        /// <summary>
        /// Try and get the specified camera rig.
        /// Should not need to use this most of the time.
        /// </summary>
        /// <returns>The camera rig, or null if not found or not initialized</returns>
        /// <param name="cameraId">Camera identifier.</param>
        public CameraRig GetCameraRig(CameraId cameraId) {
            if (!_initialized) {
                return null;
            }

            CameraRig rig = null;

            if (_cameraRigs.TryGetValue(cameraId, out rig)) {
                return rig;
            }

            return null;
        }

        /// <summary>
        /// Get and set texture downloading.
        /// If texture download callback is connected then that will start being called
        /// </summary>
        /// <value><c>true</c> if enable capture; otherwise, <c>false</c>.</value>
        public bool EnableCapture {
            get {
                return _captureFrames;
            }
            set {
                _captureFrames = value;

                foreach (var kvp in _cameraRigs) {
                    kvp.Value.EnableCapture = value;
                }
            }
        }

        /// <summary>
        /// Gets a value indicating whether this instance is initialized.
        /// </summary>
        /// <value><c>true</c> if this instance is initialized; otherwise, <c>false</c>.</value>
        public bool IsInitialized {
            get {
                return _initialized;
            }
        }

        /// <summary>
        /// Set all cameras to a fraction of the original size.
        /// Values should be in the range of 0-1
        /// </summary>
        /// <param name="fractionOfOriginalSize">Fraction of original size.</param>
        public void SetCameraResolutions(float fractionOfOriginalSize) {
            foreach (var rig in _cameraRigs.Values) {
                rig.SetResolution(fractionOfOriginalSize);
            }
        }

        /// <summary>
        /// Set specified camera to a fraction of the original size
        /// Values should be in the range of 0-1
        /// </summary>
        /// <param name="cameraId">Camera identifier.</param>
        /// <param name="fractionOfOriginalSize">Fraction of original size.</param>
        public void SetCameraResolution(CameraId cameraId, float fractionOfOriginalSize) {
            CameraRig rig;

            if (_cameraRigs.TryGetValue(cameraId, out rig)) {
                rig.SetResolution(fractionOfOriginalSize);
            }
        }

        /// <summary>
        /// Set all cameras to a scaled size using the specifed new height
        /// </summary>
        /// <param name="height">Height.</param>
        public void SetCameraResolutions(int height) {
            foreach (var rig in _cameraRigs.Values) {
                if (rig.DistortedCameraImage.height != height) {
                    rig.SetResolution (height);
                }
            }
        }

        /// <summary>
        /// Set the specified camera to the new scaled resolution based on the new height
        /// </summary>
        /// <param name="cameraId">Camera identifier.</param>
        /// <param name="height">Height.</param>
        public void SetCameraResolution(CameraId cameraId, int height) {
            CameraRig rig;

            if (_cameraRigs.TryGetValue(cameraId, out rig)) {
                rig.SetResolution(height);
            }
        }

        /// <summary>
        /// Get the resolution of the specified camera
        /// </summary>
        /// <returns>The resolution.</returns>
        /// <param name="cameraId">Camera identifier.</param>
        public Size CameraResolution(CameraId cameraId) {
            CameraRig rig;

            if (_cameraRigs.TryGetValue(cameraId, out rig)) {
                return rig.Calibration.Model.ImageSize;
            }

            return Size.Zero;
        }

        /// <summary>
        /// Get all camera resolutions
        /// </summary>
        /// <returns>The resolutions.</returns>
        public Dictionary<CameraId, Size> CameraResolutions() {
            var dic = new Dictionary<CameraId, Size>();

            foreach (var rig in _cameraRigs.Values) {
                dic[rig.cameraId] = rig.Calibration.Model.ImageSize;
            }

            return dic;
        }

        /// <summary>
        /// Get an array of all the available cameras, enabled or not
        /// </summary>
        /// <value>The available cameras.</value>
        public CameraId[] AvailableCameras {
            get {
                return _cameraRigs.Keys.ToArray();
            }
        }

        /// <summary>
        /// Get an array of all the enabled cameras (ones that are rendering)
        /// </summary>
        /// <value>The enabled cameras.</value>
        public CameraId[] EnabledCameras {
            get {
                var enabled = new List<CameraId>();

                foreach (var kvp in _cameraRigs) {
                    if (kvp.Value.CameraEnabled) {
                        enabled.Add(kvp.Key);
                    }
                }

                return enabled.ToArray();
            }
        }

        /// <summary>
        /// Check if the specified camera is enabled
        /// </summary>
        /// <returns><c>true</c> if camera is enabled; otherwise, <c>false</c>.</returns>
        /// <param name="cameraId">Camera identifier.</param>
        public bool IsCameraEnabled(CameraId cameraId) {
            var rig = GetCameraRig(cameraId);

            if (rig != null) {
                return rig.CameraEnabled;
            }

            return false;
        }

        /// <summary>
        /// Enable or disable the specified camera
        /// </summary>
        /// <param name="cameraId">Camera identifier.</param>
        /// <param name="enable">If set to <c>true</c> enable.</param>
        public void EnableCamera(CameraId cameraId, bool enable) {
            var rig = GetCameraRig(cameraId);

            if (rig != null) {
                rig.CameraEnabled = enable;
                ZippyEventManager.SendEvent (ZippyEventType.EnabledCameras, EnabledCameras);
            }
        }

        /// <summary>
        /// Enable or disable all the cameras
        /// </summary>
        /// <param name="enable">If set to <c>true</c> enable.</param>
        public void EnableCameras(bool enable) {
            foreach (var kvp in _cameraRigs) {
                kvp.Value.CameraEnabled = enable;
            }

            ZippyEventManager.SendEvent (ZippyEventType.EnabledCameras, EnabledCameras);
        }

        /// <summary>
        /// Enable or disable the specifed camera's depth image
        /// </summary>
        /// <param name="cameraId">Camera identifier.</param>
        /// <param name="enable">If set to <c>true</c> enable.</param>
        public void EnableDepthImage(CameraId cameraId, bool enable) {
            var rig = GetCameraRig(cameraId);

            if (rig != null) {
                rig.DepthImageEnabled = enable;
            }
        }

        /// <summary>
        /// Is the depth image enabled on the the specified camera
        /// </summary>
        /// <returns><c>true</c> if the camera depth image is enabled; otherwise, <c>false</c>.</returns>
        /// <param name="cameraId">Camera identifier.</param>
        public bool IsDepthImageEnabled(CameraId cameraId) {
            var rig = GetCameraRig(cameraId);

            if (rig != null) {
                return rig.DepthImageEnabled;
            }

            return false;
        }

        /// <summary>
        /// Enable or disable all the depth images
        /// </summary>
        /// <param name="enable">If set to <c>true</c> enable.</param>
        public void EnableDepthImages(bool enable) {
            foreach (var kvp in _cameraRigs) {
                kvp.Value.DepthImageEnabled = enable;
            }
        }

        /// <summary>
        /// Get a list of cameras with enabled depth images
        /// </summary>
        /// <value>The enabled depth images.</value>
        public CameraId[] EnabledDepthImages {
            get {
                var enabled = new List<CameraId>();

                foreach (var kvp in _cameraRigs) {
                    if (kvp.Value.DepthImageEnabled) {
                        enabled.Add(kvp.Key);
                    }
                }

                return enabled.ToArray();
            }
        }

        /// <summary>
        /// Enable or disable the specifed camera's xyz image
        /// </summary>
        /// <param name="cameraId">Camera identifier.</param>
        /// <param name="enable">If set to <c>true</c> enable.</param>
        public void EnableXYZImage(CameraId cameraId, bool enable) {
            var rig = GetCameraRig(cameraId);

            if (rig != null) {
                rig.XYZImageEnabled = enable;
            }
        }

        /// <summary>
        /// Is the xyz image enabled on the the specified camera
        /// </summary>
        /// <returns><c>true</c> if the camera xyz image is enabled; otherwise, <c>false</c>.</returns>
        /// <param name="cameraId">Camera identifier.</param>
        public bool IsXYZImageEnabled(CameraId cameraId) {
            var rig = GetCameraRig(cameraId);

            if (rig != null) {
                return rig.XYZImageEnabled;
            }

            return false;
        }

        /// <summary>
        /// Enable or disable all the xyz images
        /// </summary>
        /// <param name="enable">If set to <c>true</c> enable.</param>
        public void EnableXYZImages(bool enable) {
            foreach (var kvp in _cameraRigs) {
                kvp.Value.XYZImageEnabled = enable;
            }
        }

        /// <summary>
        /// Get a list of cameras with enabled depth images
        /// </summary>
        /// <value>The enabled depth images.</value>
        public CameraId[] EnabledXYZImages {
            get {
                var enabled = new List<CameraId>();

                foreach (var kvp in _cameraRigs) {
                    if (kvp.Value.XYZImageEnabled) {
                        enabled.Add(kvp.Key);
                    }
                }

                return enabled.ToArray();
            }
        }

        /// <summary>
        /// Set the callback for receiving the downloaded images.
        /// Callbacks occur on the rendering thread.
        /// Callbacks should conform to the ProcessImageFuncPtr defiend in the zippy_image_interop.h C++ header file
        /// </summary>
        /// <param name="processingCallback">Processing callback.</param>
        public void AddDownloadedImageCallback(System.IntPtr processingCallback) {
            TextureDownloaderWrapper.AddImageProcessingCallback(processingCallback);
        }

        /// <summary>
        /// Removes the downloaded image callback.
        /// </summary>
        public void RemoveDownloadedImageCallback(System.IntPtr processingCallback) {
            TextureDownloaderWrapper.RemoveImageProcessingCallback(processingCallback);
        }

        /// <summary>
        /// Coroutine to update the downloads every frame
        /// </summary>
        /// <returns>The downloads.</returns>
        IEnumerator UpdateDownloads() {
            while (true) {
                yield return new WaitForEndOfFrame();
                TextureDownloaderWrapper.UpdateDownloads();
            }
        }

        void Update() {
            TextureDownloaderWrapper.UpdateDownloads();
        }
    }
}
