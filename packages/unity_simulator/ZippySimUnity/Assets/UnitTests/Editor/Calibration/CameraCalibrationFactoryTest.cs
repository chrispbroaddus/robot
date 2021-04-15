using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Zippy;
using NUnit.Framework;
using NUnit;

namespace Zippy {
    [TestFixture]
    public class CameraCalibrationFactoryTest {
        //TODO rewrite test now we are using JSON files

        //        static readonly string SAMPLE_CAMERA_CALIBRATION_FILE = "Assets/UnitTests/Data/sample_cameras.xml";
        //
        //        void CheckLeftCamera(CameraCalibration c)
        //        {
        //            Assert.AreEqual("Left", c.Name);
        //            Assert.AreEqual(0, c.Index);
        //            Assert.AreEqual("508380442542", c.SerialNumber);
        //            Assert.AreEqual(CameraModelType.calibu_fu_fv_u0_v0_k1_k2_k3, c.Model.ModelType);
        //            Assert.AreEqual(1920, c.Model.OriginalImageSize.width);
        //            Assert.AreEqual(1080, c.Model.OriginalImageSize.height);
        //            Assert.AreEqual(1920, c.Model.ImageSize.width);
        //            Assert.AreEqual(1080, c.Model.ImageSize.height);
        //            var parameters = new float[] { 1053.945f, 1054.555f, 949.6166f, 543.7125f, 0.05052828f, -0.04035575f, -0.003082343f};
        //            Assert.AreEqual(parameters[0], c.Model.FocalLength.x, float.Epsilon);
        //            Assert.AreEqual(parameters[1], c.Model.FocalLength.y, float.Epsilon);
        //            Assert.AreEqual(parameters[2], c.Model.CameraCenter.x, float.Epsilon);
        //            Assert.AreEqual(parameters[3], c.Model.CameraCenter.y, float.Epsilon);
        //
        //            Assert.IsTrue(c.Model is Poly3Camera);
        //            var p = c.Model as Poly3Camera;
        //            Assert.AreEqual(parameters[4], p.K1, float.Epsilon);
        //            Assert.AreEqual(parameters[5], p.K2, float.Epsilon);
        //            Assert.AreEqual(parameters[6], p.K3, float.Epsilon);
        //
        //            //TODO T and R
        //        }
        //
        //        void CheckRightCamera(CameraCalibration c)
        //        {
        //            Assert.AreEqual("Right", c.Name);
        //            Assert.AreEqual(1, c.Index);
        //            Assert.AreEqual("501182442542", c.SerialNumber);
        //            Assert.AreEqual(CameraModelType.calibu_fu_fv_u0_v0_k1_k2_k3, c.Model.ModelType);
        //            Assert.AreEqual(1920, c.Model.OriginalImageSize.width);
        //            Assert.AreEqual(1080, c.Model.OriginalImageSize.height);
        //            Assert.AreEqual(1920, c.Model.ImageSize.width);
        //            Assert.AreEqual(1080, c.Model.ImageSize.height);
        //            var parameters = new float[] { 1055.987f, 1056.976f, 946.1303f, 528.6812f, 0.0245075f, -0.01127254f, -0.01241093f };
        //            Assert.AreEqual(parameters[0], c.Model.FocalLength.x, float.Epsilon);
        //            Assert.AreEqual(parameters[1], c.Model.FocalLength.y, float.Epsilon);
        //            Assert.AreEqual(parameters[2], c.Model.CameraCenter.x, float.Epsilon);
        //            Assert.AreEqual(parameters[3], c.Model.CameraCenter.y, float.Epsilon);
        //
        //            Assert.IsTrue(c.Model is Poly3Camera);
        //            var p = c.Model as Poly3Camera;
        //            Assert.AreEqual(parameters[4], p.K1, float.Epsilon);
        //            Assert.AreEqual(parameters[5], p.K2, float.Epsilon);
        //            Assert.AreEqual(parameters[6], p.K3, float.Epsilon);
        //
        //            //TODO T and R
        //        }
        //
        //        [Test]
        //        public void LoadSampleCameras()
        //        {
        //            var cameras = CameraCalibrationFactory.Load(SAMPLE_CAMERA_CALIBRATION_FILE);
        //
        //            Assert.NotNull(cameras);
        //            Assert.AreEqual(2, cameras.Length);
        //
        //            CheckLeftCamera(cameras[0]);
        //            CheckRightCamera(cameras[1]);
        //        }
        //
        //        [Test]
        //        public void ScaleCamera()
        //        {
        //            var cameras = CameraCalibrationFactory.Load(SAMPLE_CAMERA_CALIBRATION_FILE);
        //
        //            var cam = cameras[0];
        //
        //            float scale = 0.5f;
        //            cam.Model.Scale(scale);
        //
        //            Assert.AreEqual(1920, cam.Model.OriginalImageSize.width);
        //            Assert.AreEqual(1080, cam.Model.OriginalImageSize.height);
        //            Assert.AreEqual(1920 * scale, cam.Model.ImageSize.width);
        //            Assert.AreEqual(1080 * scale, cam.Model.ImageSize.height);
        //
        //            Assert.AreEqual(1053.945f * scale, cam.Model.FocalLength.x, float.Epsilon);
        //            Assert.AreEqual(1054.555f * scale, cam.Model.FocalLength.y, float.Epsilon);
        //            Assert.AreEqual((949.6166f + 0.5f) * scale - 0.5f, cam.Model.CameraCenter.x, float.Epsilon);
        //            Assert.AreEqual((543.7125f + 0.5f) * scale - 0.5f, cam.Model.CameraCenter.y, float.Epsilon);
        //        }
    }
}
