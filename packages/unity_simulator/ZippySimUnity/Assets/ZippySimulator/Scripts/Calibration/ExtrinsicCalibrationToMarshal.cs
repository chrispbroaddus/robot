using System.Runtime.InteropServices;

namespace Zippy {
    [StructLayout(LayoutKind.Sequential)]
    public struct ExtrinsicCalibrationToMarshal {
        public float rodriguesx;
        public float rodriguesy;
        public float rodriguesz;
        public float translationx;
        public float translationy;
        public float translationz;
    }
}