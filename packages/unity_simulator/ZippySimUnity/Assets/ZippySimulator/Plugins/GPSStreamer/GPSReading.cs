using System.Runtime.InteropServices;

namespace Zippy {
    [StructLayout(LayoutKind.Sequential)]
    public struct GPSReading {
        public double latitude;
        public double longitude;
        public double altitude;
        public double timestamp;
    }
}
