using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;

namespace Zippy {
    public static class VehicleCalibrationJsonExporterWrapper {

        public static int initialize(
            string filename,
            float fWrtCTx, float fWrtCTy, float fWrtCTz, float fWrtCQx, float fWrtCQy, float fWrtCQz, float fWrtCQw,
            float jpWrtFTx, float jpWrtFTy, float jpWrtFTz, float jpWrtFQx, float jpWrtFQy, float jpWrtFQz, float jpWrtFQw,
            float distBetweenWheel) {
            return VehicleCalibrationJsonExporter_export (
                       filename,
                       fWrtCTx, fWrtCTy, fWrtCTz, fWrtCQx, fWrtCQy,  fWrtCQz,  fWrtCQw,
                       jpWrtFTx,  jpWrtFTy, jpWrtFTz,  jpWrtFQx, jpWrtFQy,  jpWrtFQz,  jpWrtFQw,
                       distBetweenWheel);
        }

        [DllImport("simulated_zippy", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern int VehicleCalibrationJsonExporter_export(
            string filename,
            float fWrtCTx, float fWrtCTy, float fWrtCTz, float fWrtCQx, float fWrtCQy, float fWrtCQz, float fWrtCQw,
            float jpWrtFTx, float jpWrtFTy, float jpWrtFTz, float jpWrtFQx, float jpWrtFQy, float jpWrtFQz, float jpWrtFQw,
            float distBetweenWheel);
    }
}
