using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

public static class GlogWrapper {
    public static void Init() {
        //plugin checks and ensures multiple glog inits do not happen, so glog does not seg fault the app. checking in plugin allows safe use in the editor
        Simulator_InitGLOG (System.Environment.GetCommandLineArgs()[0]);
    }

    [DllImport("simulated_zippy", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
    private static extern void Simulator_InitGLOG (string appName);
}
