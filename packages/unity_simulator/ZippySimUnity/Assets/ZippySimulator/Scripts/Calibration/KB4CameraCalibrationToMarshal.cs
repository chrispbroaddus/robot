using System.Runtime.InteropServices;

namespace Zippy {
    [StructLayout(LayoutKind.Sequential)]
    public struct KB4CameraCalibrationToMarshal {
        public int resolutionX;
        public int resolutionY;
        public float focalLengthX;
        public float focalLengthY;
        public float centerX;
        public float centerY;
        public float kb4_0;
        public float kb4_1;
        public float kb4_2;
        public float kb4_3;
    }
}