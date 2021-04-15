using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Zippy {
    /// <summary>
    /// Distorted mesh.
    /// Create a mesh whose UV coordinates are distorted based on the results from a callback.
    /// Used to simulate lens distortion
    /// </summary>
    [RequireComponent(typeof(MeshRenderer), typeof(MeshFilter))]
    public class DistortedMesh : MonoBehaviour {
        public delegate Vector2 UVPosition(Vector2 uvPoint);    //Callback delegate
        MeshRenderer _renderer;
        MeshFilter _filter;

        /// <summary>
        /// Create the mesh with the specified rows, cols, aspect ratio and callback.
        /// </summary>
        /// <param name="rows">Rows.</param>
        /// <param name="cols">Cols.</param>
        /// <param name="heightByWidthAspectRatio">Height by width aspect ratio.</param>
        /// <param name="uvPositionCallback">Uv position callback.</param>
        public void Create(int rows, int cols, float heightByWidthAspectRatio, UVPosition uvPositionCallback, Texture2D validPixelsMask, UVPosition validPixelMaskUVCallback) {
            if (rows < 2 || cols < 2) {
                throw new System.ArgumentException("rows and cols must be 2 or greater");
            }

            if (heightByWidthAspectRatio <= 0) {
                throw new System.ArgumentException("heightByWidthAspectRatio must be greater than 0");
            }

            if (uvPositionCallback == null) {
                throw new System.ArgumentNullException("uvPositionCallback cannot be null");
            }

            var mesh = new Mesh();
            float uDelta = 1.0f / (float)(cols - 1);
            float vDelta = 1.0f / (float)(rows - 1);
            float colDelta = uDelta;
            float rowDelta = heightByWidthAspectRatio * vDelta;
            //to make pivot in the center of the mesh
            Vector2 startOffset = new Vector2(- 0.5f, - 0.5f * heightByWidthAspectRatio);
            var vertices = new Vector3[rows * cols];
            var uvs = new Vector2[rows * cols];
            var uv2s = new Vector2[rows * cols];
            int verticesIndex = 0;

            for (int rr = 0; rr < rows; rr++) {
                for (int cc = 0; cc < cols; cc++) {
                    vertices[verticesIndex] = new Vector3(startOffset.x + colDelta * cc, startOffset.y + rowDelta * rr, 0);
                    var uv = new Vector2 (uDelta * cc, vDelta * rr);
                    uvs[verticesIndex] = uvPositionCallback(uv);
                    uv2s [verticesIndex] = validPixelMaskUVCallback(uv);
                    verticesIndex++;
                }
            }

            mesh.vertices = vertices;
            mesh.uv = uvs;
            mesh.uv2 = uv2s;
            int numTriangles = (rows - 1) * (cols - 1) * 6;
            var triangles = new int[numTriangles];
            int triangleIndex = 0;

            for (int rr = 0; rr < rows - 1; rr++) {
                for (int cc = 0; cc < cols - 1; cc++) {
                    var bottomLeft = rr * cols + cc;
                    var topLeft = bottomLeft + cols;
                    var bottomRight = bottomLeft + 1;
                    var topRight = topLeft + 1;
                    triangles[triangleIndex++] = bottomLeft;
                    triangles[triangleIndex++] = topLeft;
                    triangles[triangleIndex++] = topRight;
                    triangles[triangleIndex++] = bottomLeft;
                    triangles[triangleIndex++] = topRight;
                    triangles[triangleIndex++] = bottomRight;
                }
            }

            mesh.triangles = triangles;

            if (_renderer == null) {
                _renderer = GetComponent<MeshRenderer>();
                var shader = Shader.Find("Zippy/Lens/DistortedMesh");
                _renderer.material = new Material(shader);
            }

            _renderer.material.SetTexture ("_ValidMask", validPixelsMask);

            if (_filter == null) {
                _filter = GetComponent<MeshFilter>();
            }

            mesh.RecalculateNormals ();
            _filter.mesh = mesh;
        }

        /// <summary>
        /// Gets or sets the texture.
        /// </summary>
        /// <value>The texture.</value>
        public Texture Texture {
            get {
                return _renderer.material.mainTexture;
            }
            set {
                _renderer.material.mainTexture = value;
            }
        }
    }
}
