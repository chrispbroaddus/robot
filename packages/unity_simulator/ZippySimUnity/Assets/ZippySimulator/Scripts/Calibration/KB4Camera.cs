using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Threading;

namespace Zippy {
    public class KB4Camera : CameraModel {
        const float MAX_SQUARE_INVERTABLE_DISTANCE = 1e-8f;
        const float MASK_MAX_SQUARE_INVERTABLE_DISTANCE = 0.5f;

        public KB4Camera(int width, int height, float[] parameters, float zoom = 1.0f)
        : base(width, height, parameters, 8, zoom) {
            Init();
        }
        #region implemented abstract members of CameraModel

        public override Vector2 UnprojectImageToCalibratedPoint(Vector2 pxPoint) {
            float fu = FocalLength.x;
            float fv = FocalLength.y;
            float u0 = CameraCenter.x;
            float v0 = CameraCenter.y;
            float k0 = K1;
            float k1 = K2;
            float k2 = K3;
            float k3 = K4;
            float un = pxPoint.x - u0;
            float vn = pxPoint.y - v0;
            float psi = Mathf.Atan2( fu * vn, fv * un );
            float rth = un / (fu * Mathf.Cos(psi) );
            // Use Newtons method to solve for theta.
            float th = rth;

            for (int i = 0; i < 5; i++) {
                // f = (th + k0*th**3 + k1*th**5 + k2*th**7 + k3*th**9 - rth)^2
                float th2 = th * th;
                float th3 = th2 * th;
                float th4 = th2 * th2;
                float th6 = th4 * th2;
                float x0 = k0 * th3 + k1 * th4 * th + k2 * th6 * th + k3 * th6 * th3 - rth + th;
                float x1 = 3 * k0 * th2 + 5 * k1 * th4 + 7 * k2 * th6 + 9 * k3 * th6 * th2 + 1;
                float d  = 2 * x0 * x1;
                float d2 = 4 * th * x0 * (3 * k0 + 10 * k1 * th2 + 21 * k2 * th4 + 36 * k3 * th6) + 2 * x1 * x1;
                float delta = d / d2;
                th -= delta;
            }

            Vector2 calibratedPoint = new Vector2();
            float z = Mathf.Cos(th);
            calibratedPoint.x = Mathf.Sin(th) * Mathf.Cos(psi) / z;
            calibratedPoint.y = Mathf.Sin(th) * Mathf.Sin(psi) / z;
            return calibratedPoint;
        }

        Vector2 ProjectCalibratedPointToImage(Vector2 calibratedPoint) {
            var fu = FocalLength.x;
            var fv = FocalLength.y;
            var u0 = CameraCenter.x;
            var v0 = CameraCenter.y;
            var k0 = K1;
            var k1 = K2;
            var k2 = K3;
            var k3 = K4;
            var theta = Mathf.Atan2( calibratedPoint.magnitude, 1 );
            var psi = Mathf.Atan2( calibratedPoint.y, calibratedPoint.x );
            var theta2 = theta * theta;
            var theta3 = theta2 * theta;
            var theta5 = theta3 * theta2;
            var theta7 = theta5 * theta2;
            var theta9 = theta7 * theta2;
            var r = theta + k0 * theta3 + k1 * theta5 + k2 * theta7 + k3 * theta9;
            var pix = new Vector2();
            pix.x = fu * r * Mathf.Cos(psi) + u0;
            pix.y = fv * r * Mathf.Sin(psi) + v0;
            return pix;
        }

        object _locker = new object();

        protected override void CalculateCalibratedPlane (out Vector2 topLeft, out Vector2 bottomRight) {
            ValidPixelMask = new Texture2D (ImageSize.width, ImageSize.height, TextureFormat.Alpha8, false, false);
            byte[] bytes = new byte[ImageSize.width * ImageSize.height];
            var pcQueue = new ProducerConsumerQueue (SystemInfo.processorCount);
            Vector2 tl = Vector2.zero, br = Vector2.zero;

            //scan every pixel to find which are valid
            //there is a lot of wasted computation here, but simple to implement.
            for (int row = 0; row < ImageSize.height; row++ ) {
                int rr = row;   //have to make local copy, because row may be incremented before the enqued command runs
                pcQueue.EnqueueItem (() => {
                    CalculateCalibratedPlaneRow(rr, ref bytes, ref tl, ref br);
                });
            }

            pcQueue.Shutdown (true);
            topLeft = tl / Zoom;
            bottomRight = br / Zoom;
            //mark up dead zone
            var calibratedPlaneSize = bottomRight - topLeft;
            Vector2 topLeftCopy = topLeft;
            pcQueue = new ProducerConsumerQueue (SystemInfo.processorCount);

            for (int row = 0; row < ImageSize.height; row++ ) {
                int rr = row;   //have to make local copy, because row may be incremented before the enqued command runs
                pcQueue.EnqueueItem (() => {
                    CalculateCalibratedPlaneRowDeadZone(rr, ref bytes, topLeftCopy, calibratedPlaneSize);
                });
            }

            pcQueue.Shutdown (true);
            ValidPixelMask.LoadRawTextureData (bytes);
            ValidPixelMask.Apply ();
        }

        void CalculateCalibratedPlaneRow(int row, ref byte[] bytes, ref Vector2 topLeft, ref Vector2 bottomRight) {
            Vector2 px = new Vector2 (0, row);
            int byteRow = row * ImageSize.width;
            Vector2 tl = Vector2.zero;
            Vector2 br = Vector2.zero;

            for (int col = 0; col < ImageSize.width; col++) {
                px.x = col;
                Vector2 calibratedPoint = UnprojectImageToCalibratedPoint (px);
                Vector2 px2 = ProjectCalibratedPointToImage (calibratedPoint);
                float sqrMag = Vector2.SqrMagnitude (px2 - px);
                int bytePosition = byteRow + col;
                //need a more leanient check on creating mask or get dead zones in the image.
                bytes [bytePosition] = sqrMag > MASK_MAX_SQUARE_INVERTABLE_DISTANCE ? (byte)0 : (byte)255;

                //need a strict check or plane ends up massive
                if (sqrMag > MAX_SQUARE_INVERTABLE_DISTANCE) {
                    continue;
                }

                tl.x = Mathf.Min (tl.x, calibratedPoint.x);
                tl.y = Mathf.Min (tl.y, calibratedPoint.y);
                br.x = Mathf.Max (br.x, calibratedPoint.x);
                br.y = Mathf.Max (br.y, calibratedPoint.y);
            }

            if (tl.x < topLeft.x ||
                    tl.y < topLeft.y ||
                    br.x > bottomRight.x ||
                    br.y > bottomRight.y) {
                lock (_locker) {
                    topLeft.x = Mathf.Min (topLeft.x, tl.x);
                    topLeft.y = Mathf.Min (topLeft.y, tl.y);
                    bottomRight.x = Mathf.Max (bottomRight.x, br.x);
                    bottomRight.y = Mathf.Max (bottomRight.y, br.y);
                }
            }
        }

        void CalculateCalibratedPlaneRowDeadZone(int row, ref byte[] bytes, Vector2 topLeft, Vector2 calibratedPlaneSize) {
            Vector2 px = new Vector2 (0, row);
            int byteRow = row * ImageSize.width;
            Vector2 tl = Vector2.zero;
            Vector2 br = Vector2.zero;

            for (int col = 0; col < ImageSize.width; col++) {
                int bytePosition = byteRow + col;

                if (bytes[bytePosition] == 0) {
                    continue;
                }

                px.x = col;
                Vector2 calibratedPoint = UnprojectImageToCalibratedPoint (px);

                if (!IsCalibratedPointOnCalibratedPlaneTexture(calibratedPoint, topLeft, calibratedPlaneSize)) {
                    bytes [bytePosition] = (byte)128;
                }
            }
        }

        public bool IsCalibratedPointOnCalibratedPlaneTexture(Vector2 calibratedPoint, Vector2 topLeft, Vector2 calibratedPlaneSize) {
            var x = (calibratedPoint.x - topLeft.x) / calibratedPlaneSize.x;
            var y = 1.0f - ((calibratedPoint.y - topLeft.y) / calibratedPlaneSize.y);
            //Debug.Log(calibratedPoint + " : " + x + " , " + y);
            return (x >= 0 && x <= 1.0f && y >= 0 && y <= 1.0f);
        }


        #endregion

        /// <summary>
        /// Gets k1 distortion factor.
        /// </summary>
        /// <value>The k1.</value>
        public float K1 {
            get {
                return Parameters[0];
            }
        }

        /// <summary>
        /// Gets k2 distortion factor.
        /// </summary>
        /// <value>The k2.</value>
        public float K2 {
            get {
                return Parameters[1];
            }
        }

        /// <summary>
        /// Gets k3 distortion factor.
        /// </summary>
        /// <value>The k3.</value>
        public float K3 {
            get {
                return Parameters[2];
            }
        }

        /// <summary>
        /// Gets k4 distortion factor.
        /// </summary>
        /// <value>The k4.</value>
        public float K4 {
            get {
                return Parameters[3];
            }
        }
    }
}
