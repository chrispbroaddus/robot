#if (UNITY_EDITOR_LINUX || UNITY_STANDALONE_LINUX) && !(UNITY_EDITOR_OSX || UNITY_EDITOR_WIN)
#define IMAGE_SAVER_SUPPORTED
#endif

    using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;

namespace Zippy {
    public static class JoystickSubscriberWrapper {
        public static int initialize(string address, int timeoutInMilliseconds, int numJoystickAxis, double maxLatencyInSec) {
            return JoystickSubscriber_initialize (address, timeoutInMilliseconds, numJoystickAxis, maxLatencyInSec);
        }

        public static JoystickAxisResponse receive() {
            return JoystickSubscriber_receive ();
        }

        [DllImport("simulated_zippy", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern int JoystickSubscriber_initialize(string address, int timeoutInMilliseconds, int numJoystickAxis, double maxLatencyInSec);

        [DllImport("simulated_zippy")]
        public static extern JoystickAxisResponse JoystickSubscriber_receive();

    }
}