using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

namespace Zippy {
    /// <summary>
    /// User interface manager.
    /// Used in the Demo_FullConents scene to control the cameras and view their images
    /// </summary>
    public class UIManagerFullContents : MonoBehaviour {
        public Toggle writeToDiskToggle;                //Toggle used to toggle image saving
        public CameraRigImageViewer cameraImageView;    //Viewer to see the camera image
        public CameraRigDepthViewer cameraDepthView;    //Viewer to see the depth image
        public Text activeCameraLabel;                  //Label to show the name of the active camera
        public Text activeCameraResolutionLabel;        //Label to show the resolution of the active camera
        public InputField resolutionInputField;         //Input field to set the height of the active/all cameras
        public Button applyButton;                      //Apply button to apply the new resolution
        public Toggle allCamerasToggle;                 //Toggle to set whether to set the resolution of all cameras or just the active one
        public Text gpsStatusLabel;                     //GPS status label
        public Text imuStatusLabel;                     //imu status label

        CameraId _activeCamera;
        CameraId[] _allCameraIds;
        int _activeCameraIndex = -1;

        void OnEnable() {
            ZippyEventManager.StartListening (ZippyEventType.CamerasReady, HandleZippyEvents);
            ZippyEventManager.StartListening (ZippyEventType.EnabledCameras, HandleZippyEvents);
            ZippyEventManager.StartListening (ZippyEventType.GPSData, HandleZippyEvents);
            ZippyEventManager.StartListening (ZippyEventType.IMUData, HandleZippyEvents);
        }

        void OnDisable() {
            ZippyEventManager.StopListening (ZippyEventType.CamerasReady, HandleZippyEvents);
            ZippyEventManager.StopListening (ZippyEventType.EnabledCameras, HandleZippyEvents);
            ZippyEventManager.StopListening (ZippyEventType.GPSData, HandleZippyEvents);
            ZippyEventManager.StopListening (ZippyEventType.IMUData, HandleZippyEvents);
        }

        void HandleZippyEvents(ZippyEventType zippyEvent, ZippyEventPayloadBase payload) {
            switch (zippyEvent) {
            case ZippyEventType.CamerasReady:
                HandleCameraRigManagerInitialized ();
                break;

            case ZippyEventType.EnabledCameras:
                var camPayload = payload as ZippyEventPayload<CameraId[]>;
                HandleEnabledCameras (camPayload.value);
                break;

            case ZippyEventType.GPSData:
                var gpsPayload = payload as ZippyEventPayload<GlobalPosition>;
                HandleGPSUpdate (gpsPayload.value);
                break;

            case ZippyEventType.IMUData:
                var imuPayload = payload as ZippyEventPayload<IMUReading[]>;
                HandleImuUpdate (imuPayload.value);
                break;

            default:
                break;
            }
        }

        void HandleCameraRigManagerInitialized() {
            writeToDiskToggle.isOn = ImageSaver.Instance.EnableWriteToDisk;
            writeToDiskToggle.onValueChanged.AddListener(ToggleWriteToDisk);
            applyButton.onClick.AddListener(ApplyResolution);
        }

        void HandleEnabledCameras(CameraId[] enabledCameras) {
            _allCameraIds = enabledCameras;
            System.Array.Sort(_allCameraIds);

            if (_allCameraIds.Length <= 0) {
                _activeCameraIndex = -1;
                activeCameraLabel.text = "None";
                activeCameraResolutionLabel.text = "-";
            }
            else {
                _activeCameraIndex = Mathf.Clamp (_activeCameraIndex, 0, _allCameraIds.Length - 1);
                SetActiveCameraView ();
            }
        }

        /// <summary>
        /// Go to the next camera
        /// </summary>
        public void NextCamera() {
            if (_allCameraIds.Length <= 0) {
                return;
            }

            _activeCameraIndex++;

            if (_activeCameraIndex >= _allCameraIds.Length) {
                _activeCameraIndex = 0;
            }

            SetActiveCameraView();
        }

        /// <summary>
        /// Go to the previous camera
        /// </summary>
        public void PreviousCamera() {
            if (_allCameraIds.Length <= 0) {
                return;
            }

            _activeCameraIndex--;

            if (_activeCameraIndex < 0) {
                _activeCameraIndex = _allCameraIds.Length - 1;
            }

            SetActiveCameraView();
        }

        void SetActiveCameraView() {
            _activeCamera = _allCameraIds[_activeCameraIndex];
            cameraImageView.SelectedCamera = _activeCamera;
            cameraDepthView.SelectedCamera = _activeCamera;
            activeCameraLabel.text = _activeCamera.ToString();
            activeCameraResolutionLabel.text = CameraRigManager.Instance.CameraResolution(_activeCamera).ToString();
        }

        void ToggleCapture(bool enableCapture) {
            CameraRigManager.Instance.EnableCapture = enableCapture;
        }

        void ToggleWriteToDisk(bool enableWrite) {
            ImageSaver.Instance.EnableWriteToDisk = enableWrite;
        }

        void ToggleStreamToNetwork(bool enableStream) {
            ImageStreamer.Instance.EnableStreamToNetwork = enableStream;
        }

        void ApplyResolution() {
            string value = resolutionInputField.text;
            int height = 0;

            if (!int.TryParse(value, out height)) {
                Debug.LogError("Failed to parse height");
                return;
            }

            if (allCamerasToggle.isOn) {
                CameraRigManager.Instance.SetCameraResolutions(height);
            }
            else {
                CameraRigManager.Instance.SetCameraResolution(_activeCamera, height);
            }

            activeCameraResolutionLabel.text = CameraRigManager.Instance.CameraResolution(_activeCamera).ToString();
        }

        void HandleGPSUpdate(GlobalPosition gp) {
            gpsStatusLabel.text = gp.ToString ();
        }

        void HandleImuUpdate(IMUReading[] imu) {
            if (imu == null || imu.Length <= 0) {
                imuStatusLabel.text = "";
            }
            else {
                imuStatusLabel.text = imu[imu.Length - 1].ToString ();
            }
        }

    }
}
