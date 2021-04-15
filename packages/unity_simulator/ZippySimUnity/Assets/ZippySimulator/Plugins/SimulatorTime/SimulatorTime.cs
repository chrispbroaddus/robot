using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;

public static class SimulatorTime {
    //Get the current 'GPS' time as a count in nanoseconds
    //Only really useful for passing to native plugins that want this
    public static ulong GPSSystemTimeCount {
        get {
            return SimulatorTime_gpsSystemtimeCount ();
        }
    }

    //Convienience function for adding seconds in float format to a gps time in nanosecond count format
    //Useful for workign out times during physics simulation updates
    public static ulong AddSeconds(ulong gpsSystemTimeCount, float seconds) {
        return SimulatorTime_addSeconds (gpsSystemTimeCount, seconds);
    }

    [DllImport("simulated_zippy")]
    private static extern ulong SimulatorTime_gpsSystemtimeCount();

    [DllImport("simulated_zippy")]
    private static extern ulong SimulatorTime_addSeconds(ulong gpsSystemtimeCount, float seconds);

}
