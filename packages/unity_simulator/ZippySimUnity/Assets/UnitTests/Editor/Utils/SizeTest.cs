using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using NUnit.Framework;
using NUnit;

namespace Zippy {
    [TestFixture]
    public class SizeTest {
        [Test]
        public void DefaultCreation() {
            var s = new Size();
            Assert.AreEqual(0, s.width);
            Assert.AreEqual(0, s.height);
        }

        [Test]
        public void SpecifiedCreation() {
            var s1 = new Size(10, 35);
            Assert.AreEqual(10, s1.width);
            Assert.AreEqual(35, s1.height);
            var s2 = new Size(-10, -35);
            Assert.AreEqual(-10, s2.width);
            Assert.AreEqual(-35, s2.height);
        }

        [Test]
        public void ZeroCreation() {
            var s = Size.Zero;
            Assert.AreEqual(0, s.width);
            Assert.AreEqual(0, s.height);
        }

        [Test]
        public void SettingValues() {
            var s = new Size();
            s.width = 12;
            s.height = 34;
            Assert.AreEqual(12, s.width);
            Assert.AreEqual(34, s.height);
        }

        [Test]
        public void Equal() {
            var s1 = new Size(12, 34);
            var s2 = new Size(12, 34);
            var s3 = new Size(11, 34);
            var s4 = new Size(11, 32);
            var s5 = new Size(55, 44);
            var s6 = s2;
            Assert.AreEqual(s1, s2);
            Assert.AreEqual(s1, s6);
            Assert.AreNotEqual(s1, s3);
            Assert.AreNotEqual(s1, s4);
            Assert.AreNotEqual(s1, s5);
        }
    }
}
