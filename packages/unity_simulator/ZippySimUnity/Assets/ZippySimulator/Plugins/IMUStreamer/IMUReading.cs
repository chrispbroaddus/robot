using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;

namespace Zippy {
    [StructLayout(LayoutKind.Sequential)]
    public struct IMUReading {
        public Vector3 accel;
        public Vector3 gyro;
        public ulong gpsSystemtimeCount;
        public IMUReading(Vector3 accelerometer, Vector3 gyroscope, ulong timestamp) {
            accel = accelerometer;
            gyro = gyroscope;
            gpsSystemtimeCount = timestamp;
        }

        public override string ToString () {
            return "accel: " + accel.ToString () + ", gyro: " + gyro.ToString () + ", timestamp: " + gpsSystemtimeCount;
        }

    }
}
