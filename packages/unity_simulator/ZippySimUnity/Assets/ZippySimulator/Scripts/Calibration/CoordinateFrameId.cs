using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Zippy {
    /// <summary>
    /// Coordinate Frame identifiers for coordinate frames on the Zippy robot.
    /// Changes to this must also be reflected in the hal::coordinate_frame_id.proto IDL.
    /// </summary>
    public enum CoordinateFrameId {
        FrontLeftStereo = 0,
        FrontRightStereo = 1,
        RearLeftStereo = 2,
        RearRightStereo = 3,
        FrontFisheye = 4,
        RearFisheye = 5,
        LeftFisheye = 6,
        RightFisheye = 7,
        BaseLink = 8,
    };

}
