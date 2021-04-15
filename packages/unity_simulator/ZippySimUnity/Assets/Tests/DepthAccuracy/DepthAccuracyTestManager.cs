using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;
using Zippy;

namespace Zippy.Test {
    [StructLayout(LayoutKind.Sequential)]
    public struct CameraImage {
        public CameraId cameraId;
        public ImageType imageType;
        public System.IntPtr data;
        public int width;
        public int height;
        public int bytesPerPixel;
        public int frameNumber;
        public float timestamp;
    };

    public class DepthAccuracyTestManager : MonoBehaviour {
        public Transform target;
        public float minTestDistanceMeters = 0.01f;
        public float maxTestDistanceMeters = 60.0f;
        public float incrementMeters = 0.1f;
        public float desiredAccuracyMeters = 0.0001f;

        Vector3 _currentPosition;
        static Dictionary<int, float> _expectedDepthAtFrameNumber = new Dictionary<int, float>();

        static CameraRig[] _rigs;
        static float _desiredAccuracyMeters;

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ProcessImageDelegate(CameraImage cameraImage);

        System.IntPtr _intptrDelegate;

        void OnEnable() {
            _rigs = null;
            _expectedDepthAtFrameNumber.Clear();
            _desiredAccuracyMeters = desiredAccuracyMeters;
            _currentPosition = new Vector3(0, 0, minTestDistanceMeters);

            if (CameraRigManager.Instance.IsInitialized) {
                HandleInitialized();
            }
            else {
                CameraRigManager.Instance.OnInitialized += HandleInitialized;
            }

            ProcessImageDelegate callback_delegate = new ProcessImageDelegate(ImageProcessor);
            _intptrDelegate = Marshal.GetFunctionPointerForDelegate(callback_delegate);
            CameraRigManager.Instance.EnableCapture = true;
            CameraRigManager.Instance.AddDownloadedImageCallback(_intptrDelegate);
        }

        void OnDisable() {
            if (CameraRigManager.Instance != null) {
                CameraRigManager.Instance.OnInitialized -= HandleInitialized;
            }

            CameraRigManager.Instance.RemoveDownloadedImageCallback(_intptrDelegate);
        }

        void HandleInitialized() {
            _rigs = FindObjectsOfType<CameraRig>();

            foreach (var rig in _rigs) {
                rig.transform.position = Vector3.zero;
                rig.transform.rotation = Quaternion.identity;
            }

            StartCoroutine(DepthTest());
        }

        IEnumerator DepthTest() {
            while (_currentPosition.z <= maxTestDistanceMeters) {
                var pos = _currentPosition;
                target.position = pos;
                _expectedDepthAtFrameNumber[Time.frameCount] = pos.z;
                yield return null;
                pos.z -= desiredAccuracyMeters;
                target.position = pos;
                _expectedDepthAtFrameNumber[Time.frameCount] = pos.z;
                yield return null;
                pos = _currentPosition;
                pos.z += desiredAccuracyMeters;
                _expectedDepthAtFrameNumber[Time.frameCount] = pos.z;
                yield return null;
                _currentPosition.z += incrementMeters;
            }
        }

        static void ImageProcessor(CameraImage cameraImage) {
            if (cameraImage.imageType != ImageType.Depth) {
                return;
            }

            Debug.Log(cameraImage.frameNumber);
            float[] image = new float[cameraImage.width * cameraImage.height];
            Marshal.Copy(cameraImage.data, image, 0, image.Length);
            float expected = _expectedDepthAtFrameNumber[cameraImage.frameNumber];
            float depth = image[image.Length / 2];
            float delta = Mathf.Abs(expected - depth);

            if (delta > _desiredAccuracyMeters) {
                Debug.Log(cameraImage.cameraId + " : " + cameraImage.frameNumber + ": " + expected + " = " + depth + " : delta " + delta);
            }
        }
    }

}
