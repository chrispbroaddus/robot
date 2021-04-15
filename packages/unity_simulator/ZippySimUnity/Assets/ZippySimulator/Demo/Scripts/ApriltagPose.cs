using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Zippy {
    [System.Serializable]
    /// <summary>
    /// Apriltag pose.
    /// </summary>
    public struct ApriltagPoseMetadata {
        public string family;
        public int id;
        public int border;
        public float side_length_in_meters;
        public float rodrigues_x;
        public float rodrigues_y;
        public float rodrigues_z;
        public float translation_x;
        public float translation_y;
        public float translation_z;
        public GameObject gameObject;
        //      TargetCoordinate targetCoordinate;
    }
}

