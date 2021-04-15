using System.Collections;
using UnityEngine;
using UnityEditor;
using NUnit.Framework;

namespace Zippy {
    [TestFixture]
    public class GlobalPositioningTest {
        [Test]
        public void DistanceBetweenTwoLocations() {
            GlobalPosition[] from = new GlobalPosition[] {
                new GlobalPosition (37.95, 122.95, 0),
                new GlobalPosition (0, 0, 0),
                new GlobalPosition (20, 179, 0),
            };
            GlobalPosition[] to = new GlobalPosition[] {
                new GlobalPosition (38, 123, 0),
                new GlobalPosition (-1, 1, 0),
                new GlobalPosition (20, -179, 0),
            };
            double[] expectedDist = new double[] {7087.3548335118803, 157425.53710841201, 209210.96547189332 };

            for (int ii = 0; ii < from.Length; ii++) {
                var d = GlobalPositioning.DistanceBetweenTwoLocations (from [ii], to [ii]);
                Assert.AreEqual (expectedDist [ii], d, double.Epsilon);
            }
        }

        [Test]
        public void BearingBetweenTwoLocations() {
            GlobalPosition[] from = new GlobalPosition[] {
                new GlobalPosition (37.95, 122.95, 0),
                new GlobalPosition (0, 0, 0),
                new GlobalPosition (20, 179, 0),
            };
            GlobalPosition[] to = new GlobalPosition[] {
                new GlobalPosition (38, 123, 0),
                new GlobalPosition (-1, 1, 0),
                new GlobalPosition (20, -179, 0),
            };
            double[] expectedBearing = new double[] {38.232601177976321, 135.00436354465515, 89.657949187616694 };

            for (int ii = 0; ii < from.Length; ii++) {
                var b = GlobalPositioning.BearingBetweenTwoLocations (from [ii], to [ii]);
                Assert.AreEqual (expectedBearing [ii], b, 1e-12);
            }
        }

        [Test]
        public void GlobalPositionFromUnityPositionUsingBearingAndDistance() {
            var refGlobalPosition = new GlobalPosition (37.95, 122.95, 0);
            var refPosition = new Vector3[] {
                new Vector3(0, 0, 0),
                new Vector3(10, 1, 20),
                new Vector3(-10, -1, -20)
            };
            double distance = 1000.0;
            var bearing = new double[] { 0, 45, 90, 135, 180, 225, 270, 315, 360 };
            double altitude = 100.0;
            var expectedLat = new double[] {
                37.958983152841199,
                37.9563520482905,
                37.9500,
                37.9436479517096,
                37.9410168471588,
                37.9436479517096,
                37.9500,
                37.9563520482905,
                37.958983152841199,
            };
            var expectedLon = new double[] {
                122.9500,
                122.958055724168,
                122.961392021839,
                122.958055027693,
                122.9500,
                122.941944972307,
                122.938607978161,
                122.941944275832,
                122.9500,
            };
            var expectedAlt = new double[] {
                altitude - refPosition [0].y,
                altitude - refPosition [1].y,
                altitude - refPosition [2].y,
            };

            for (int rp = 0; rp < refPosition.Length; rp++) {
                for (int ii = 0; ii < bearing.Length; ii++) {
                    var gp = GlobalPositioning.GlobalPositionFromUnityPosition (refGlobalPosition, refPosition [rp], bearing [ii] * GlobalPositioning.DEG_TO_RAD, distance, altitude);
                    Assert.AreEqual (expectedLat[ii], gp.LatitudeInDegrees, 1e-12);
                    Assert.AreEqual (expectedLon[ii], gp.LongitudeInDegrees, 1e-12);
                    Assert.AreEqual (expectedAlt[rp], gp.AltitudeInMeters, 1e-12);
                }
            }
        }

        [Test]
        public void GlobalPositionFromUnityPositionUsingPosition() {
            var refGlobalPosition = new GlobalPosition (37.95, 122.95, 100);
            var refPosition = new Vector3[] {
                new Vector3(0, 0, 0),
                new Vector3(10, 1, 20),
                new Vector3(-10, -1, -20)
            };
            float distance = 1000.0f;
            var bearing = new float[] { 0, 45, 90, 135, 180, 225, 270, 315, 360 };
            float altitude = 100.0f;
            var expectedLat = new double[] {
                37.958983152841199,
                37.9563520482905,
                37.9500,
                37.9436479517096,
                37.9410168471588,
                37.9436479517096,
                37.9500,
                37.9563520482905,
                37.958983152841199,
            };
            var expectedLon = new double[] {
                122.9500,
                122.958055724168,
                122.961392021839,
                122.958055027693,
                122.9500,
                122.941944972307,
                122.938607978161,
                122.941944275832,
                122.9500,
            };

            for (int rp = 0; rp < refPosition.Length; rp++) {
                for (int ii = 0; ii < bearing.Length; ii++) {
                    //create a position by rotating a north vector (0, 0, 1), scaling by distance and adding it to the ref position
                    float bearingRad = bearing [ii] * Mathf.Deg2Rad;
                    var offset = new Vector3(Mathf.Sin(bearingRad) * distance, altitude, Mathf.Cos(bearingRad) * distance);
                    var position = refPosition [rp] + offset;
                    var gp = GlobalPositioning.GlobalPositionFromUnityPosition (refGlobalPosition, refPosition [rp], position);
                    Assert.AreEqual (expectedLat[ii], gp.LatitudeInDegrees, 1e-8);
                    Assert.AreEqual (expectedLon[ii], gp.LongitudeInDegrees, 1e-8);
                    Assert.AreEqual (altitude, gp.AltitudeInMeters, 1e-8);
                }
            }
        }
    }
}
