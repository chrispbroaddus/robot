using System;
using UnityEngine;
using UnityEditor;
using NUnit.Framework;

namespace Zippy {
    [TestFixture]
    public class GlobalPositionTest {
        [Test]
        public void ValueCreation() {
            var t1 = Time.realtimeSinceStartup;
            var gp = new GlobalPosition (10, 20, 30);
            var t2 = Time.realtimeSinceStartup;
            Assert.AreEqual (10, gp.LatitudeInDegrees, double.Epsilon);
            Assert.AreEqual (20, gp.LongitudeInDegrees, double.Epsilon);
            Assert.AreEqual (30, gp.AltitudeInMeters, double.Epsilon);
            Assert.GreaterOrEqual (gp.Timestamp, t1);
            Assert.GreaterOrEqual (t2, gp.Timestamp);
        }

        [Test]
        public void StringCreation() {
            var t1 = Time.realtimeSinceStartup;
            var gp = new GlobalPosition ("10, 20, 30");
            var t2 = Time.realtimeSinceStartup;
            Assert.AreEqual (10, gp.LatitudeInDegrees, double.Epsilon);
            Assert.AreEqual (20, gp.LongitudeInDegrees, double.Epsilon);
            Assert.AreEqual (30, gp.AltitudeInMeters, double.Epsilon);
            Assert.GreaterOrEqual (gp.Timestamp, t1);
            Assert.GreaterOrEqual (t2, gp.Timestamp);
        }

        [Test]
        public void StringCreationFailure() {
            Assert.Throws (typeof(System.ArgumentException), () => {
                new GlobalPosition ("10");
            });
            Assert.Throws (typeof(System.ArgumentException), () => {
                new GlobalPosition ("10, 20");
            });
            Assert.Throws (typeof(System.ArgumentException), () => {
                new GlobalPosition ("foo,20,30");
            });
            Assert.Throws (typeof(System.ArgumentException), () => {
                new GlobalPosition ("10,foo,30");
            });
            Assert.Throws (typeof(System.ArgumentException), () => {
                new GlobalPosition ("10, 20, foo");
            });
            Assert.Throws (typeof(System.ArgumentException), () => {
                new GlobalPosition ("10, 20, 30, 40");
            });
            Assert.Throws (typeof(System.ArgumentNullException), () => {
                new GlobalPosition ("");
            });
        }

        [Test]
        public void Radians() {
            double latDeg = 10;
            double lonDeg = 20;
            double latRad = latDeg * Math.PI / 180.0;
            double lonRad = lonDeg * Math.PI / 180.0;
            var gp = new GlobalPosition (latDeg, lonDeg, 30);
            Assert.AreEqual (latRad, gp.LatitudeInRadians, double.Epsilon);
            Assert.AreEqual (lonRad, gp.LongitudeInRadians, double.Epsilon);
        }

        [Test]
        public void InRange() {
            double[] lats = new double[] { -90, -45, 0, 45, 90 };
            double[] lons = new double[] { -180, -90, 0, 90, 180 };

            for (int ii = 0; ii < lats.Length; ii++) {
                var gp = new GlobalPosition (lats [ii], lons [ii], 30);
                Assert.AreEqual (lats [ii], gp.LatitudeInDegrees, double.Epsilon);
                Assert.AreEqual (lons [ii], gp.LongitudeInDegrees, double.Epsilon);
            }
        }

        [Test]
        public void OutOfRange() {
            double[] lats = new double[] { -91, 91, 500, -500, 0, 0, 0, 0 };
            double[] lons = new double[] { 0, 0, 0, 0, -181, 181, 500, -500 };

            for (int ii = 0; ii < lats.Length; ii++) {
                Assert.Throws (typeof(System.ArgumentOutOfRangeException), () => {
                    new GlobalPosition (lats [ii], lons [ii], 30);
                });
            }
        }

        [Test]
        public void Convert() {
            var gp = new GlobalPosition (10, 20, 30);
            var reading = gp.ConvertToGPSReading ();
            Assert.AreEqual (gp.LatitudeInDegrees, reading.latitude, double.Epsilon);
            Assert.AreEqual (gp.LongitudeInDegrees, reading.longitude, double.Epsilon);
            Assert.AreEqual (gp.AltitudeInMeters, reading.altitude, double.Epsilon);
            Assert.AreEqual (gp.Timestamp, reading.timestamp, double.Epsilon);
        }
    }
}