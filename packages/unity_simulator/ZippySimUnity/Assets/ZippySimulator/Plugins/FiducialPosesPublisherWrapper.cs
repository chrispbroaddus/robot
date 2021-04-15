using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;

namespace Zippy {
    /// <summary>
    /// Fiducial poses publisher wrapper.
    /// </summary>
    public static class FiducialPosesPublisherWrapper {

        public static int initialize(string addr, int highWaterMark, int lingerPeriodInMilliseconds) {
            return FiducialPosesPublisher_initializeApriltagPosePublisher (addr, highWaterMark, lingerPeriodInMilliseconds);
        }

        public static void add(TargetCoordinate targetCoordinate, string poseName, int fiducialId, Quaternion tagToTargetQuat, Vector3 tagToTargetPos) {
            FiducialPose fiducialPose;
            fiducialPose.translation_x = tagToTargetPos.x;
            fiducialPose.translation_y = tagToTargetPos.y;
            fiducialPose.translation_z = tagToTargetPos.z;
            var r = Zippy.Geometry.QuaternionToRodrigues (tagToTargetQuat);// * quatWrtXaxis90);
            fiducialPose.rodrigues_x = r.x;
            fiducialPose.rodrigues_y = r.y;
            fiducialPose.rodrigues_z = r.z;
            FiducialPosesPublisher_add (targetCoordinate, poseName, fiducialId, fiducialPose);
        }

        public static void clear() {
            FiducialPosesPublisher_clear ();
        }

        public static bool send() {
            return FiducialPosesPublisher_send () == 1 ? true : false;
        }

        public static void stop() {
            FiducialPosesPublisher_stop ();
        }

        [DllImport("simulated_zippy", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern int FiducialPosesPublisher_initializeApriltagPosePublisher (string addr, int highWaterMark, int lingerPeriodInMilliseconds);

        [DllImport("simulated_zippy", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern void FiducialPosesPublisher_add (TargetCoordinate targetCoordinate, string poseName, int fiducialId, FiducialPose fiducialPose);

        [DllImport("simulated_zippy")]
        private static extern void FiducialPosesPublisher_clear ();

        [DllImport("simulated_zippy")]
        private static extern int FiducialPosesPublisher_send ();

        [DllImport("simulated_zippy")]
        private static extern void FiducialPosesPublisher_stop ();

    }

}
