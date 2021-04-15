using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

namespace Zippy {
    public class Geometry {
        /// <summary>
        /// Rodrigueses to quaternion.
        /// </summary>
        /// <returns>The to quaternion.</returns>
        /// <param name="x">The x coordinate.</param>
        /// <param name="y">The y coordinate.</param>
        /// <param name="z">The z coordinate.</param>
        static public Quaternion RodriguesToQuaternion(Vector3 r) {
            var theta = r.magnitude;
            r /= theta;
            var c = Mathf.Cos (theta / 2);
            var s = Mathf.Sin (theta / 2);
            Quaternion q = new Quaternion (r.x * s, r.y * s, r.z * s, c);
            return q;
        }

        /// <summary>
        /// Rodrigueses to quaternion.
        /// </summary>
        /// <returns>The to quaternion.</returns>
        /// <param name="x">The x coordinate.</param>
        /// <param name="y">The y coordinate.</param>
        /// <param name="z">The z coordinate.</param>
        static public Vector3 QuaternionToRodrigues(Quaternion q) {
            Vector3 r = new Vector3 ();
            float thetaOverTwo = Mathf.Acos (q.w);
            r.x = q.x / Mathf.Sin (thetaOverTwo);
            r.y = q.y / Mathf.Sin (thetaOverTwo);
            r.z = q.z / Mathf.Sin (thetaOverTwo);
            r *= thetaOverTwo * 2;
            return r;
        }


        /// <summary>
        /// Inverses the transform.
        /// </summary>
        /// <returns>The transform.</returns>
        /// <param name="qo">Qo.</param>
        /// <param name="to">To.</param>
        /// <param name="qi">Qi.</param>
        /// <param name="ti">Ti.</param>
        public static void InvertTransform(out Quaternion qo, out Vector3 to, Quaternion qi, Vector3 ti) {
            qo = qi;
            qo.x = -qo.x;
            qo.y = -qo.y;
            qo.z = -qo.z;
            to = -(qo * ti);
        }

        /// <summary>
        /// Concatenates the transform.
        /// </summary>
        /// <returns>The transform.</returns>
        /// <param name="qo">Qo.</param>
        /// <param name="to">To.</param>
        /// <param name="q0">Q0.</param>
        /// <param name="t0">T0.</param>
        /// <param name="q1">Q1.</param>
        /// <param name="t1">T1.</param>
        public static void ConcatenateTransform(out Quaternion qo, out Vector3 to, Quaternion q0, Vector3 t0, Quaternion q1, Vector3 t1) {
            qo = q0 * q1;
            to = q0 * t1 + t0;
        }
    }
}

