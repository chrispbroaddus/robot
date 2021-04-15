using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;

namespace Zippy {
    public static class SimulatedVCUWrapper {

        public static void initialize(string address, float PIDKu, float PIDTu, VehicleCalibration zippyBotCalibration) {
            SimulatedVCU_initialize (address, PIDKu, PIDTu, zippyBotCalibration);
        }

        public static bool isConnected() {
            bool ret = SimulatedVCU_isConnected () != 0;
            return ret;
        }

        public static void stop() {
            SimulatedVCU_stop();
        }

        public static void setWheelTelemetry(WheelTelemetry wheelTelemetry) {
            SimulatedVCU_setWheelTelemetry(wheelTelemetry);
        }

        public static VehicleControlCommand getVehicleControlCommand() {
            return SimulatedVCU_getVehicleControlCommand();
        }

        [DllImport("simulated_zippy", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern void SimulatedVCU_initialize(string address, float PIDKu, float PIDTu, VehicleCalibration zippyBotCalibration);

        [DllImport("simulated_zippy")]
        public static extern int SimulatedVCU_isConnected();

        [DllImport("simulated_zippy")]
        public static extern void SimulatedVCU_stop();

        [DllImport("simulated_zippy")]
        public static extern void SimulatedVCU_setWheelTelemetry(WheelTelemetry wheelTelemetry);

        [DllImport("simulated_zippy")]
        public static extern VehicleControlCommand SimulatedVCU_getVehicleControlCommand();

    }

}
