using System.Runtime.InteropServices;

namespace Zippy {
    [StructLayout(LayoutKind.Sequential)]
    public struct FiducialPose {
        public float rodrigues_x;
        public float rodrigues_y;
        public float rodrigues_z;
        public float translation_x;
        public float translation_y;
        public float translation_z;
    };
}