using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Zippy {
    [System.Serializable]
    /// <summary>
    /// Axle info per wheel.
    /// </summary>
    public struct WheelAxleInfo {
        public WheelCollider wheel;
        public Transform cylinder;
        public bool motor; // is this wheel attached to motor?
        public bool steering; // does this wheel apply steer angle?
    }
}
