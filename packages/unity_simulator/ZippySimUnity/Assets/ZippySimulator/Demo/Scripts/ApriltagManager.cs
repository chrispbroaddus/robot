using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

namespace Zippy {

    /// <summary>
    /// AprilTag manager.
    /// </summary>
    public class ApriltagManager : SingletonBase<ApriltagManager> {

        public Camera m_frontFisheyeCamera;
        public Camera m_rearFisheyeCamera;
        public Camera m_leftFisheyeCamera;
        public Camera m_rightFisheyeCamera;

        public Camera m_frontLeftStereo;
        public Camera m_frontRightStereo;
        public Camera m_rearLeftStereo;
        public Camera m_rearRightStereo;

        public bool m_readFromConfigurationFile = false;

        private List<ApriltagPoseMetadata> m_poses = new List<ApriltagPoseMetadata> ();
        private List<TargetCoordinate> m_targetCoordinates = new List<TargetCoordinate> ();
        private List<GameObject> m_gameobjects = new List<GameObject>();

        private string m_aprilTagRootPath = "https://s3-us-west-1.amazonaws.com/zippysimulator/assets/fiducials/apriltags/";

        private bool _initialized = false;

        /// <summary>
        /// Occurs when initialized.
        /// </summary>
        public event System.Action OnInitialized;

        /// <summary>
        /// Run when the instance is created.
        /// </summary>
        private void Start() {
            FiducialPosesPublisherWrapper.initialize(
                ConnectionsReader.FullAddress(ConnectionTopic.fiducial_poses),
                SimulatorSettingsManager.Settings.networking.zmqHighWaterMark,
                SimulatorSettingsManager.Settings.networking.zmqLingerTimeMs);
            m_targetCoordinates.Add (TargetCoordinate.FRONT_FISHEYE_CAM);
            _initialized = true;

            if (OnInitialized != null) {
                OnInitialized ();
            }
        }

        protected override void OnDestroy() {
            FiducialPosesPublisherWrapper.stop();
            base.OnDestroy ();
        }

        /// <summary>
        /// Perform an update every frame
        /// </summary>
        /// <returns>The update.</returns>
        private void Update() {
            PublishPoses ();
        }

        /// <summary>
        /// Loads the apriltag on scene.
        /// </summary>
        /// <returns>The apriltag on scene.</returns>
        public IEnumerator AddApriltagOnExistingPlane(string apriltagFamily, int apriltagId, int border, Transform child) {
            if (apriltagFamily != "AprilTag36h11" && apriltagFamily != "36h11") {
                UnityEngine.Debug.Log ("36h11 is the only supported AprilTag family.");
            }

            UnityEngine.Debug.Log ("Drawing Apriltags...");
            ApriltagPoseMetadata pose = new ApriltagPoseMetadata();
            pose.id = apriltagId;
            pose.border = 0;
            Vector3 rod = Geometry.QuaternionToRodrigues (child.rotation);
            pose.family = "36h11";
            pose.rodrigues_x = rod.x;
            pose.rodrigues_y = rod.y;
            pose.rodrigues_z = rod.z;
            pose.side_length_in_meters = child.localScale.x;
            pose.translation_x = child.position.x;
            pose.translation_y = child.position.y;
            pose.translation_z = child.position.z;
            string apriltagfamilyFolder;
            string apriltagPrefix;
            int numBitOnSide;
            apriltagfamilyFolder = "36h11/";
            apriltagPrefix = "tag36_11_";
            numBitOnSide = 6;
            string fileName = pose.id.ToString("D5") + ".png";
            string fullUrl = m_aprilTagRootPath + apriltagfamilyFolder + apriltagPrefix + fileName;
            float desiredScale = pose.side_length_in_meters * ((float)numBitOnSide + (float)pose.border * 2) / (float)numBitOnSide;
            child.transform.localScale = new Vector3 (desiredScale, desiredScale, desiredScale);
            Texture2D tex;
            tex = new Texture2D(1, 1, TextureFormat.DXT1, false);
            WWW www = new WWW(fullUrl);
            yield return www;
            www.LoadImageIntoTexture(tex);
            child.GetComponent<Renderer>().material.mainTexture = tex;
            m_poses.Add (pose);
            m_gameobjects.Add (child.gameObject);
        }


        /// <summary>
        /// Loads the target coorindate transform.
        /// </summary>
        /// <returns>The target coorindate transform.</returns>
        /// <param name="q">Q.</param>
        /// <param name="t">T.</param>
        /// <param name="targetCoordinate">Target coordinate.</param>
        private void LoadTargetCoorindateTransform(out Quaternion q, out Vector3 t, TargetCoordinate targetCoordinate) {
            switch (targetCoordinate) {
            case TargetCoordinate.FRONT_FISHEYE_CAM:
                q = m_frontFisheyeCamera.transform.rotation;
                t = m_frontFisheyeCamera.transform.position;
                break;

            case TargetCoordinate.REAR_FISHEYE_CAM:
                q = m_rearFisheyeCamera.transform.rotation;
                t = m_rearFisheyeCamera.transform.position;
                break;

            case TargetCoordinate.LEFT_FISHEYE_CAM:
                q = m_leftFisheyeCamera.transform.rotation;
                t = m_leftFisheyeCamera.transform.position;
                break;

            case TargetCoordinate.RIGHT_FISHEYE_CAM:
                q = m_rightFisheyeCamera.transform.rotation;
                t = m_rightFisheyeCamera.transform.position;
                break;

            case TargetCoordinate.FRONT_LEFT_STEREO_CAM:
                q = m_frontLeftStereo.transform.rotation;
                t = m_frontLeftStereo.transform.position;
                break;

            case TargetCoordinate.FRONT_RIGHT_STEREO_CAM:
                q = m_frontRightStereo.transform.rotation;
                t = m_frontRightStereo.transform.position;
                break;

            case TargetCoordinate.REAR_LEFT_STEREO_CAM:
                q = m_rearLeftStereo.transform.rotation;
                t = m_rearLeftStereo.transform.position;
                break;

            case TargetCoordinate.REAR_RIGHT_STEREO_CAM:
                q = m_rearRightStereo.transform.rotation;
                t = m_rearRightStereo.transform.position;
                break;

            case TargetCoordinate.UNITY_WORLD:
            default:
                q = Quaternion.identity;
                t = new Vector3(0f, 0f, 0f);
                break;
            }
        }

        /// <summary>
        /// Publishs the poses.
        /// </summary>
        /// <returns>The poses.</returns>
        private void PublishPoses() {
            FiducialPosesPublisherWrapper.clear ();
            int nPoses = 0;

            for (int i = 0; i < m_poses.Count; i++) {
                var pose = m_poses [i];

                foreach (var targetCoordinate in m_targetCoordinates) {
                    Transform tagToWorld = m_gameobjects [i].transform;
                    Quaternion targetToWorldQuat = new Quaternion();
                    Vector3 targetToWorldPos = new Vector3();
                    LoadTargetCoorindateTransform(out targetToWorldQuat, out targetToWorldPos, targetCoordinate);
                    Quaternion worldToTargetQuat = new Quaternion ();
                    Vector3 worldToTargetPos = new Vector3 ();
                    Zippy.Geometry.InvertTransform (out worldToTargetQuat, out worldToTargetPos, targetToWorldQuat, targetToWorldPos);
                    Quaternion tagToTargetQuat = new Quaternion();
                    Vector3 tagToTargetPos = new Vector3();
                    Zippy.Geometry.ConcatenateTransform (out tagToTargetQuat, out tagToTargetPos,
                                                         worldToTargetQuat, worldToTargetPos,
                                                         tagToWorld.rotation, tagToWorld.position);
                    FiducialPosesPublisherWrapper.add (targetCoordinate, pose.family, pose.id, tagToTargetQuat, tagToTargetPos);
                    nPoses++;
                }
            }

            if (nPoses > 0) {
                FiducialPosesPublisherWrapper.send ();
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

    }

}

