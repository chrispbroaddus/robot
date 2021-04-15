using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

namespace Zippy {

    /// <summary>
    /// AprilTag manager.
    /// </summary>
    public class DockingStationManager : SingletonBase<DockingStationManager> {

        public List<GameObject> m_dockingStations;

        private bool _initialized = false;

        /// <summary>
        /// Run when the instance is created.
        /// </summary>
        private void Start() {
            Debug.Log ("Starting DockingStationManager...");

            if (!ApriltagManager.Instance.IsInitialized) {
                ApriltagManager.Instance.OnInitialized += Init;
                // waiting until apriltagmanager initialized.
            }
            else {
                Init ();
            }
        }

        private void Init() {
            Debug.Log ("LoadApriltagOnDockingStations...");
            StartCoroutine (LoadApriltagOnDockingStations ());
            //var ie = LoadApriltagOnDockingStations ();
            Debug.Log ("ExportStationCalibration...");
            ExportStationCalibration ();
            DockingStationJsonExporterWrapper.export("/tmp/zippy_simulator_station_list.json");
            _initialized = true;
            Debug.Log ("DockingStationManager... Initialized.");
            //yield return ie;
        }

        /// <summary>
        /// Loads the apriltag on scene.
        /// </summary>
        /// <returns>The apriltag on scene.</returns>
        private IEnumerator LoadApriltagOnDockingStations() {
            for (int i = 0; i < m_dockingStations.Count; i++) {
                foreach (Transform child in m_dockingStations[i].transform) {
                    if (child.name.Length < 8 || child.name.Substring (0, 8) != "Apriltag") {
                        continue;
                    }

                    yield return ApriltagManager.Instance.AddApriltagOnExistingPlane ("36h11",
                                 Convert.ToInt32 (child.name.Substring (8)),
                                 0,
                                 child);
                }
            }
        }

        private void ExportStationCalibration() {
            for (int i = 0; i < m_dockingStations.Count; i++) {
                int stationId = i;
                DockingStationJsonExporterWrapper.createDockingStation (stationId);
                GameObject targetDockingStation = m_dockingStations [i];
                Transform anchorPoint = targetDockingStation.transform.Find ("AnchorPoint");
                Quaternion quatAnchorWrtWorld;
                Vector3 posAnchorWrtWorld;
                Zippy.Geometry.InvertTransform (out quatAnchorWrtWorld, out posAnchorWrtWorld, anchorPoint.rotation, anchorPoint.position);

                foreach (Transform child in targetDockingStation.transform) {
                    if (child.name.Length < 8 || child.name.Substring (0, 8) != "Apriltag") {
                        continue;
                    }

                    var apriltagId = Convert.ToInt32 (child.name.Substring (8));
                    Quaternion quatAnchorWrtApriltag;
                    Vector3 posAnchorWrtApriltag;
                    Zippy.Geometry.ConcatenateTransform (out quatAnchorWrtApriltag, out posAnchorWrtApriltag,
                                                         quatAnchorWrtWorld, posAnchorWrtWorld,
                                                         child.rotation, child.position);
                    DockingStationJsonExporterWrapper.updateDockingStation (stationId, apriltagId, quatAnchorWrtApriltag, posAnchorWrtApriltag);
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

    }
}
