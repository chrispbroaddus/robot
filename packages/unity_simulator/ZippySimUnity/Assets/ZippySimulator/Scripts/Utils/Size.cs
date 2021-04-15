using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Zippy {
    /// <summary>
    /// Size.
    /// Represents an integer width and height as a single object
    /// </summary>
    [System.Serializable]
    public struct Size {
        public int width;
        public int height;

        /// <summary>
        /// Initializes a new instance of the <see cref="Zippy.Size"/> struct.
        /// </summary>
        /// <param name="w">The width.</param>
        /// <param name="h">The height.</param>
        public Size(int w, int h) {
            width = w;
            height = h;
        }

        /// <summary>
        /// Get a Size initialized to zero
        /// </summary>
        /// <value>The zero.</value>
        public static Size Zero {
            get {
                return new Size(0, 0);
            }
        }

        /// <summary>
        /// Determines whether the specified <see cref="System.Object"/> is equal to the current <see cref="Zippy.Size"/>.
        /// </summary>
        /// <param name="obj">The <see cref="System.Object"/> to compare with the current <see cref="Zippy.Size"/>.</param>
        /// <returns><c>true</c> if the specified <see cref="System.Object"/> is equal to the current <see cref="Zippy.Size"/>;
        /// otherwise, <c>false</c>.</returns>
        public override bool Equals(object obj) {
            if (obj == null) {
                return false;
            }

            if (obj.GetType() != this.GetType()) {
                return false;
            }

            Size other = (Size)obj;
            return this.width == other.width && this.height == other.height;
        }

        /// <summary>
        /// Serves as a hash function for a <see cref="Zippy.Size"/> object.
        /// </summary>
        /// <returns>A hash code for this instance that is suitable for use in hashing algorithms and data structures such as a
        /// hash table.</returns>
        public override int GetHashCode() {
            return ToString().GetHashCode();
        }

        /// <summary>
        /// Returns a <see cref="System.String"/> that represents the current <see cref="Zippy.Size"/>.
        /// </summary>
        /// <returns>A <see cref="System.String"/> that represents the current <see cref="Zippy.Size"/>.</returns>
        public override string ToString() {
            return width + "x" + height;
        }

        /// <summary>
        /// Allow multiplication with a float to scale both width and height
        /// </summary>
        /// <param name="size">Size.</param>
        /// <param name="scale">Scale.</param>
        public static Size operator *(Size size, float scale) {
            return new Size((int)((float)size.width * scale), (int)((float)size.height * scale));
        }

        /// <summary>
        /// Allow multiplication with an int to scale both width and height
        /// </summary>
        /// <param name="size">Size.</param>
        /// <param name="scale">Scale.</param>
        public static Size operator *(Size size, int scale) {
            return new Size(size.width * scale, size.height * scale);
        }

        /// <summary>
        /// Allow multiplication with a float to scale both width and height
        /// </summary>
        /// <param name="scale">Scale.</param>
        /// <param name="size">Size.</param>
        public static Size operator *(float scale, Size size) {
            return size * scale;
        }

        /// <summary>
        /// Allow multiplication with an int to scale both width and height
        /// </summary>
        /// <param name="scale">Scale.</param>
        /// <param name="size">Size.</param>
        public static Size operator *(int scale, Size size) {
            return size * scale;
        }
    }
}
