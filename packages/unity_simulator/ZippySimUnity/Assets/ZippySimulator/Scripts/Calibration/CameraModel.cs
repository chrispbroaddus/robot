using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Zippy {
    /// <summary>
    /// Camera model.
    /// Abstract base class for all camera lens models
    /// </summary>
    public abstract class CameraModel {
        /// <summary>
        /// Gets the original size of the image.
        /// </summary>
        /// <value>The original size of the image.</value>
        public Size OriginalImageSize {
            get;
            private set;
        }

        /// <summary>
        /// Gets the size of the image.
        /// </summary>
        /// <value>The size of the image.</value>
        public Size ImageSize {
            get;
            private set;
        }

        /// <summary>
        /// Gets the parameters.
        /// </summary>
        /// <value>The parameters.</value>
        protected float[] Parameters {
            get;
            private set;
        }

        /// <summary>
        /// Gets the original focal length.
        /// </summary>
        /// <value>The length of the original focal.</value>
        public Vector2 OriginalFocalLength {
            get;
            private set;
        }

        /// <summary>
        /// Gets the focal length.
        /// </summary>
        /// <value>The length of the focal.</value>
        public Vector2 FocalLength {
            get;
            private set;
        }

        /// <summary>
        /// Gets the original camera center.
        /// </summary>
        /// <value>The original camera center.</value>
        public Vector2 OriginalCameraCenter {
            get;
            private set;
        }

        /// <summary>
        /// Gets the camera center.
        /// </summary>
        /// <value>The camera center.</value>
        public Vector2 CameraCenter {
            get;
            private set;
        }

        public Texture2D ValidPixelMask {
            get;
            protected set;
        }

        Vector2 _topLeftCalibratedPoint;
        Vector2 _bottomRightCalibratedPoint;
        Vector2 _calibratedPlaneSize;
        protected float Zoom {
            get;
            private set;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="Zippy.CameraModel"/> class.
        /// </summary>
        /// <param name="modelNode">Model node.</param>
        /// <param name="expectedParamCount">Expected parameter count.</param>
        public CameraModel(int width, int height, float[] p, int expectedParamCount, float zoom = 1.0f) {
            Size size = new Size(width, height);
            OriginalImageSize = size;
            ImageSize = size;
            Zoom = zoom;

            //TODO RDF matrix. Assume is right handed

            if (p.Length != expectedParamCount) {
                throw new System.Exception("Should have " + expectedParamCount + " parameters, not " + p.Length);
            }

            OriginalFocalLength = new Vector2(p[0], p[1]);
            FocalLength = OriginalFocalLength;
            OriginalCameraCenter = new Vector2(p[2], p[3]);
            CameraCenter = OriginalCameraCenter;
            Parameters = new float[p.Length - 4];
            System.Array.Copy(p, 4, Parameters, 0, Parameters.Length);
        }

        /// <summary>
        /// Initialize the model. Must call once construction is complete
        /// </summary>
        protected void Init() {
            _topLeftCalibratedPoint = Vector2.zero;
            _bottomRightCalibratedPoint = Vector2.zero;
            CalculateCalibratedPlane (out _topLeftCalibratedPoint, out _bottomRightCalibratedPoint);
            _calibratedPlaneSize = _bottomRightCalibratedPoint - _topLeftCalibratedPoint;
        }

        protected virtual void CalculateCalibratedPlane(out Vector2 topLeft, out Vector2 bottomRight) {
            topLeft = Vector2.zero;
            bottomRight = Vector2.zero;
            Vector2[] pxCorners = new Vector2[] {
                new Vector2(-1.5f, -1.5f),                                      //top left
                new Vector2(-1.5f, ImageSize.height + 0.5f),                    //bottom left
                new Vector2(ImageSize.width + 0.5f, -1.5f),                     //top right
                new Vector2(ImageSize.width + 0.5f, ImageSize.height + 0.5f),    //bottom right
            };

            foreach (var pxCorner in pxCorners) {
                var calPoint = UnprojectImageToCalibratedPoint(pxCorner);
                topLeft.x = Mathf.Min (topLeft.x, calPoint.x) / Zoom;
                topLeft.y = Mathf.Min (topLeft.y, calPoint.y) / Zoom;
                bottomRight.x = Mathf.Max (bottomRight.x, calPoint.x) / Zoom;
                bottomRight.y = Mathf.Max (bottomRight.y, calPoint.y) / Zoom;
            }
        }

        /// <summary>
        /// Unprojects an image point to a calibrated point.
        /// </summary>
        /// <returns>The calibrated point.</returns>
        /// <param name="pxPoint">Image point.</param>
        public abstract Vector2 UnprojectImageToCalibratedPoint(Vector2 pxPoint);

        /// <summary>
        /// Multiply a point by K inverse
        /// Pulled from calibu
        /// </summary>
        /// <returns>The inv k.</returns>
        /// <param name="pxPoint">Px point.</param>
        protected Vector2 MultInvK(Vector2 pxPoint) {
            float x = (pxPoint.x - CameraCenter.x) / FocalLength.x;
            float y = (pxPoint.y - CameraCenter.y) / FocalLength.y;
            return new Vector2(x, y);
        }

        /// Pulled from Calibu
        /// (x, y, z) -> (x, y)
        ///
        /// @param ray A 3-vector of (x, y, z)
        /// @param pix A 2-vector (x, y)
        ///
        protected Vector2 Dehomogenize(Vector3 ray) {
            return new Vector3(ray.x / ray.z, ray.y / ray.z);
        }

        /// <summary>
        /// Homogenize the specified image point.
        /// Pulled from Calibu
        /// (x, y) -> (x, y, z), with z = 1
        /// </summary>
        /// <param name="pxPoint">Px point.</param>
        protected Vector3 Homogenize(Vector2 pxPoint) {
            return new Vector3(pxPoint.x, pxPoint.y, 1);
        }

        /// <summary>
        /// Convert a point on the calibrated plane to a texture coordinate
        /// </summary>
        /// <returns>texture coordinate.</returns>
        /// <param name="calibratedPoint">Calibrated point.</param>
        public Vector2 ProjectCalibratedPointToTextureCoordinate(Vector2 calibratedPoint) {
            // Calibu coord system is x right, y down. principle point near center of plane
            // Textures are x right, y up, origin at bottom left
            // Take calibrated point value and normalize to texture coordinates
            var x = (calibratedPoint.x - _topLeftCalibratedPoint.x) / _calibratedPlaneSize.x;
            var y = 1.0f - ((calibratedPoint.y - _topLeftCalibratedPoint.y) / _calibratedPlaneSize.y);
            //ensure the coordinate in within the image
            return new Vector2(Mathf.Clamp01(x), Mathf.Clamp01(y));
        }

        public bool IsCalibratedPointOnCalibratedPlaneTexture(Vector2 calibratedPoint) {
            var x = (calibratedPoint.x - _topLeftCalibratedPoint.x) / _calibratedPlaneSize.x;
            var y = 1.0f - ((calibratedPoint.y - _topLeftCalibratedPoint.y) / _calibratedPlaneSize.y);
            return (x >= 0 && x <= 1.0f && y >= 0 && y <= 1.0f);
        }

        /// <summary>
        /// Create a projection matrix for Unity
        /// </summary>
        /// <returns>Projection matrix.</returns>
        /// <param name="nearClip">Near clip.</param>
        /// <param name="farClip">Far clip.</param>
        public Matrix4x4 CalibrationPlaneProjectionMatrix(float nearClip, float farClip) {
            Matrix4x4 proj = new Matrix4x4();
            float left = _topLeftCalibratedPoint.x * nearClip;
            float right = _bottomRightCalibratedPoint.x * nearClip;
            //top and bottom negated as calibration has y down, and unity has y up
            float top = -_topLeftCalibratedPoint.y * nearClip;
            float bottom = -_bottomRightCalibratedPoint.y * nearClip;
            proj[0, 0] = 2 * nearClip / (right - left);
            proj[0, 2] = (right + left) / (right - left);
            proj[1, 1] = 2 * nearClip / (top - bottom);
            proj[1, 2] = (top + bottom) / (top - bottom);
            proj[2, 2] = -(farClip + nearClip) / (farClip - nearClip);
            proj[2, 3] = -2 * farClip * nearClip / (farClip - nearClip);
            proj[3, 2] = -1.0f;
            return proj;
        }

        /// <summary>
        /// Calculate the required texture size for the render texture
        /// This texture is the calibrated plane, so it will be larger than the final texture
        /// if lens has distortion.
        /// For a good looking output image after distortion sampling, the render texture
        /// should be much larger than the final output size. 2x is a good starting point.
        /// </summary>
        /// <returns>The texture size.</returns>
        public Size CalibratedTextureSize(float calibratedPlaneScaleFactor) {
            //make sure the central region is of high enough resolution
            float centralRegionFraction = 0.25f;
            float w = centralRegionFraction * ImageSize.width;
            float h = centralRegionFraction * ImageSize.height;
            Vector2 topLeft = new Vector2 ((ImageSize.width - w) / 2, (ImageSize.height - h) / 2);
            Vector2 bottomRight = new Vector2 ((ImageSize.width + w) / 2, (ImageSize.height + h) / 2);
            Vector2 topLeftCal = UnprojectImageToCalibratedPoint (topLeft);
            Vector2 bottomRightCal = UnprojectImageToCalibratedPoint (bottomRight);
            var topLeftTex = ProjectCalibratedPointToTextureCoordinate (topLeftCal);
            var bottomRightTex = ProjectCalibratedPointToTextureCoordinate (bottomRightCal);
            //work out texture coord range
            float horizontal = Mathf.Abs(bottomRightTex.x - topLeftTex.x);
            float vertical = Mathf.Abs(topLeftTex.y - bottomRightTex.y);
            //work out full tex width
            int texWidth = Mathf.CeilToInt(calibratedPlaneScaleFactor * w / horizontal);
            int texHeight = Mathf.CeilToInt(calibratedPlaneScaleFactor * h / vertical);
            // TODO replace this hot fix with a proper workaround
            int maxTextureSize = SimulatorSettingsManager.Settings.cameras.maxTextureSize;
            //int maxTextureSize = SystemInfo.maxTextureSize;

            //keep within max texture size
            if (texWidth > maxTextureSize || texHeight > maxTextureSize) {
                if (texWidth > texHeight) {
                    float scale = (float)maxTextureSize / (float)texWidth;
                    texWidth = maxTextureSize;
                    texHeight = (int)(scale * (float)texHeight);
                }
                else {
                    float scale = (float)maxTextureSize / (float)texHeight;
                    texHeight = maxTextureSize;
                    texWidth = (int)(scale * (float)texWidth);
                }
            }

            return new Size(texWidth, texHeight);
        }

        /// <summary>
        /// Scale the calibration.
        /// Should be in the range of 0-1 for sane results
        /// </summary>
        /// <param name="scale">Scale.</param>
        public void Scale(float scale) {
            ImageSize = scale * OriginalImageSize;
            FocalLength = scale * OriginalFocalLength;
            Vector2 half = new Vector2(0.5f, 0.5f);
            CameraCenter = scale * (OriginalCameraCenter + half) - half;
            Init();
        }
    }
}
