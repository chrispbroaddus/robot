using System.Runtime.InteropServices;

namespace Zippy {
    [StructLayout(LayoutKind.Sequential)]
    public struct WheelTelemetry {
        public ulong gpsSystemtimeCount;
        public float centerBodyLinearVelocity;
    }
}