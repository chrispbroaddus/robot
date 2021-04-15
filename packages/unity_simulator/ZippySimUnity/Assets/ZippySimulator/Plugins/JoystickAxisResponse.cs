using System.Runtime.InteropServices;

namespace Zippy {
    [StructLayout(LayoutKind.Sequential)]
    public struct JoystickAxisResponse {
        public int success;
        public float axis0;
        public float axis1;
    }
}