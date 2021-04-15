using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Zippy {
    /// <summary>
    /// Camera identifiers for each camera on the Zippy robot.
    /// Changes to this must also be reflected in the zippy_image_interop.h plugin api header file
    /// </summary>
    public enum CameraId {
        FrontLeftStereo = 0,
        FrontRightStereo = 1,
        RearLeftStereo = 2,
        RearRightStereo = 3,
        FrontFisheye = 4,
        RearFisheye = 5,
        LeftFisheye = 6,
        RightFisheye = 7,
    };

    public static class GlobalCameraId {
        public const int NUM_CAMERAS = 8;
    }
}
