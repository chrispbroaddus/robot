using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using NUnit.Framework;
using NUnit;

namespace Zippy {
    [TestFixture]
    public class PointTest {
        [Test]
        public void DefaultCreation() {
            var p = new Point();
            Assert.AreEqual(0, p.x);
            Assert.AreEqual(0, p.y);
        }

        [Test]
        public void SpecifiedCreation() {
            var p1 = new Point(10, 35);
            Assert.AreEqual(10, p1.x);
            Assert.AreEqual(35, p1.y);
            var p2 = new Point(-10, -35);
            Assert.AreEqual(-10, p2.x);
            Assert.AreEqual(-35, p2.y);
        }

        [Test]
        public void ZeroCreation() {
            var p = Point.Zero;
            Assert.AreEqual(0, p.x);
            Assert.AreEqual(0, p.y);
        }

        [Test]
        public void SettingValues() {
            var p = new Point();
            p.x = 12;
            p.y = 34;
            Assert.AreEqual(12, p.x);
            Assert.AreEqual(34, p.y);
        }

        [Test]
        public void Equal() {
            var p1 = new Point(12, 34);
            var p2 = new Point(12, 34);
            var p3 = new Point(11, 34);
            var p4 = new Point(11, 32);
            var p5 = new Point(55, 44);
            var p6 = p2;
            Assert.AreEqual(p1, p2);
            Assert.AreEqual(p1, p6);
            Assert.AreNotEqual(p1, p3);
            Assert.AreNotEqual(p1, p4);
            Assert.AreNotEqual(p1, p5);
        }
    }
}
