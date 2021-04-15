using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using NUnit.Framework;

namespace Zippy {
    [TestFixture]
    public class CalibuStringParsersTest {
        [Test]
        public void StringArrayFromString() {
            string[] arrays = new string[] {
                " [1; 0; 0 ] ",
                " [1053.945; 1054.555; 949.6166; 543.7125; 0.05052828; -0.04035575; -0.003082343 ] ",
            };
            string[][] stringArrays = new string[][] {
                new string[] { "1", " 0", " 0"},
                new string[] { "1053.945", " 1054.555", " 949.6166", " 543.7125", " 0.05052828", " -0.04035575", " -0.003082343"},
            };

            for (int ii = 0; ii < arrays.Length; ii++) {
                var res = CalibuStringParsers.StringArrayFromString(arrays[ii]);
                Assert.AreEqual(stringArrays[ii].Length, res.Length);

                for (int ee = 0; ee < res.Length; ee++) {
                    Assert.AreEqual(stringArrays[ii][ee], res[ee]);
                }
            }
        }

        [Test]
        public void FloatArrayFromString() {
            string[] arrays = new string[] {
                " [1; 0; 0 ] ",
                " [1053.945; 1054.555; 949.6166; 543.7125; 0.05052828; -0.04035575; -0.003082343 ] ",
            };
            float[][] floatArrays = new float[][] {
                new float[] { 1.0f, 0.0f, 0.0f },
                new float[] { 1053.945f, 1054.555f, 949.6166f, 543.7125f, 0.05052828f, -0.04035575f, -0.003082343f },
            };

            for (int ii = 0; ii < arrays.Length; ii++) {
                var res = CalibuStringParsers.FloatArrayFromString(arrays[ii]);
                Assert.AreEqual(floatArrays[ii].Length, res.Length);

                for (int ee = 0; ee < res.Length; ee++) {
                    Assert.AreEqual(floatArrays[ii][ee], res[ee], double.Epsilon);
                }
            }
        }

        [Test]
        public void FloatMatrixFromString() {
            string[] arrays = new string[] {
                " [ 1, 0, 0, 0; 0, 1, 0, 0; 0, 0, 1, 0 ] ",
                " [1053.945, 1054.555, 949.6166; 543.7125, 0.05052828, -0.04035575 ] ",
                " [ 0.9997142, 0.003130488, 0.02369865, 0.2515873; -0.009590008, 0.9606334, 0.2776536, 0.001558329; -0.02189653, -0.2778015, 0.9603889, 0.006108747 ] ",
            };
            float[][,] matrices = new float[][,] {
                new float[3, 4] { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 } },
                new float[2, 3] { { 1053.945f, 1054.555f, 949.6166f }, { 543.7125f , 0.05052828f, -0.04035575f} },
                new float[3, 4] { { 0.9997142f, 0.003130488f, 0.02369865f, 0.2515873f }, { -0.009590008f, 0.9606334f, 0.2776536f, 0.001558329f }, { -0.02189653f, -0.2778015f, 0.9603889f, 0.006108747f } }
            };

            for (int ii = 0; ii < arrays.Length; ii++) {
                var res = CalibuStringParsers.FloatMatrixFromString(arrays[ii], matrices[ii].GetLength(0), matrices[ii].GetLength(1));
                Assert.AreEqual(matrices[ii].Length, res.Length);

                for (int row = 0; row < res.Rank; row++) {
                    for (int col = 0; col < res.GetLength(row); col++) {
                        Assert.AreEqual(matrices[ii][row, col], res[row, col], double.Epsilon);
                    }
                }
            }
        }
    }

}