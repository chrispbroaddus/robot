using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using NUnit.Framework;
using NUnit;

namespace Zippy {
    [TestFixture]
    public class GeometryTest {
        /// <summary>
        /// Testing ConcatenateTransform() regressing to the arbitrarily chosen example.
        /// </summary>
        [Test]
        public void ConcatenateTransform_RegressionBasic() {
            Quaternion qo = new Quaternion ();
            Vector3 to = new Vector3 ();
            Quaternion q1 = new Quaternion (-0.049460f, 0.000000f, -0.247300f, 0.967676f);
            Vector3 t1 = new Vector3 (0.5f, 0.3f, 0.1f);
            Quaternion q2 = new Quaternion (-0.148442f, 0.000000f, -0.197923f, 0.968912f);
            Vector3 t2 = new Vector3 (0.8f, 0.6f, 0.4f);
            Geometry.ConcatenateTransform (out qo, out to, q1, t1, q2, t2);
            Quaternion qoExpected = new Quaternion (-0.191567f, 0.026921f, -0.431138f, 0.881305f);
            Vector3 toExpected = new Vector3 (1.49910f, 0.47907f, 0.46018f);
            Assert.IsTrue (Mathf.Abs(toExpected[0] - to[0]) < 1e-5);
            Assert.IsTrue (Mathf.Abs(toExpected[1] - to[1]) < 1e-5);
            Assert.IsTrue (Mathf.Abs(toExpected[2] - to[2]) < 1e-5);
            Assert.IsTrue (Mathf.Abs(qoExpected.x - qo.x) < 1e-5);
            Assert.IsTrue (Mathf.Abs(qoExpected.y - qo.y) < 1e-5);
            Assert.IsTrue (Mathf.Abs(qoExpected.z - qo.z) < 1e-5);
            Assert.IsTrue (Mathf.Abs(qoExpected.w - qo.w) < 1e-5);
        }

        /// <summary>
        /// Testing InvertTransform() regressing to the arbitrarily chosen example.
        /// </summary>
        [Test]
        public void InvertTransform_RegressionBasic() {
            Quaternion qo = new Quaternion ();
            Vector3 to = new Vector3 ();
            Quaternion q1 = new Quaternion (-0.049460f, 0.000000f, -0.247300f, 0.967676f);
            Vector3 t1 = new Vector3 (0.5f, 0.3f, 0.1f);
            Geometry.InvertTransform (out qo, out to, q1, t1);
            Quaternion qoExpected = new Quaternion (0.049460f, 0.000000f, 0.247300f, 0.967676f);
            Vector3 toExpected = new Vector3 (-0.29770f, -0.49157f, -0.14046f);
            Assert.IsTrue (Mathf.Abs(toExpected[0] - to[0]) < 1e-5);
            Assert.IsTrue (Mathf.Abs(toExpected[1] - to[1]) < 1e-5);
            Assert.IsTrue (Mathf.Abs(toExpected[2] - to[2]) < 1e-5);
            Assert.IsTrue (Mathf.Abs(qoExpected.x - qo.x) < 1e-5);
            Assert.IsTrue (Mathf.Abs(qoExpected.y - qo.y) < 1e-5);
            Assert.IsTrue (Mathf.Abs(qoExpected.z - qo.z) < 1e-5);
            Assert.IsTrue (Mathf.Abs(qoExpected.w - qo.w) < 1e-5);
        }

        /// <summary>
        /// Testing QuaterionToRodrigues() regressing to the arbitrarily chosen example.
        /// </summary>
        [Test]
        public void QuaternionToRodrigues_RegressionBasic() {
            Quaternion q = new Quaternion(-0.049460f, 0.000000f, -0.247300f, 0.967676f);
            var r = Geometry.QuaternionToRodrigues(q);
            Vector3 rExpected = new Vector3 (-0.09999981f , 0f , -0.499999f);
            Assert.IsTrue (Mathf.Abs(rExpected.x - r.x) < 1e-5);
            Assert.IsTrue (Mathf.Abs(rExpected.y - r.y) < 1e-5);
            Assert.IsTrue (Mathf.Abs(rExpected.z - r.z) < 1e-5);
        }

        /// <summary>
        /// Testing RodriguesToQuaternion() regressing to the arbitrarily chosen example.
        /// </summary>
        [Test]
        public void RodriguesToQuaternion_RegressionBasic() {
            Vector3 r = new Vector3 (-0.09999981f , 0f , -0.499999f);
            var q = Geometry.RodriguesToQuaternion(r);
            Quaternion qExpected = new Quaternion(-0.049460f, 0.000000f, -0.247300f, 0.967676f);
            Assert.IsTrue (Mathf.Abs(qExpected.x - q.x) < 1e-5);
            Assert.IsTrue (Mathf.Abs(qExpected.y - q.y) < 1e-5);
            Assert.IsTrue (Mathf.Abs(qExpected.z - q.z) < 1e-5);
            Assert.IsTrue (Mathf.Abs(qExpected.w - q.w) < 1e-5);
        }
    }
}