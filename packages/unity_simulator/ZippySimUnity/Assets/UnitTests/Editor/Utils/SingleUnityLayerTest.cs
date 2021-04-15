using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using NUnit.Framework;

namespace Zippy {
    [TestFixture]
    public class SingleUnityLayerTest {
        [Test]
        public void SetLayer() {
            SingleUnityLayer layer = new SingleUnityLayer();

            for (int ii = 0; ii < 32; ii++) {
                layer.LayerIndex = ii;
                Assert.AreEqual(ii, layer.LayerIndex);
            }

            layer.LayerIndex = -1;
            Assert.AreNotEqual(-1, layer.LayerIndex);
            layer.LayerIndex = 32;
            Assert.AreNotEqual(32, layer.LayerIndex);
        }

        [Test]
        public void Mask() {
            SingleUnityLayer layer = new SingleUnityLayer();

            for (int ii = 0; ii < 32; ii++) {
                layer.LayerIndex = ii;
                Assert.AreEqual(1 << ii , layer.Mask);
            }
        }
    }

}
