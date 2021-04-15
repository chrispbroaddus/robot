
using System.Collections;
using System.Collections.Generic;
using System.Reflection;
using UnityEngine;
using System.Runtime.InteropServices;

namespace Zippy {
    public static class DockingStationJsonExporterWrapper {

        public static void createDockingStation(int stationId) {
            DockingStationJsonExporter_createDockingStation (stationId);
        }

        public static void updateDockingStation(int stationId, int apriltagId, Quaternion quat, Vector3 pos) {
            //            Vector3 r = Zippy.Geometry.QuaternionToRodrigues (quat);
            // todo : hacked..
            DockingStationJsonExporter_updateDockingStation (stationId, apriltagId, 0f, 0f, 0f, pos.x, pos.y, pos.z);
        }

        public static void export(string filename) {
            DockingStationJsonExporter_export (filename);
        }

        [DllImport("simulated_zippy")]
        private static extern void DockingStationJsonExporter_createDockingStation(int stationId);

        [DllImport("simulated_zippy")]
        private static extern void DockingStationJsonExporter_updateDockingStation(int stationId, int apriltagId, float rx, float ry, float rz, float x, float y, float z);

        [DllImport("simulated_zippy", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern void DockingStationJsonExporter_export(string filename);

    }
}


