using System;

namespace Zippy {
    /// <summary>
    /// Point.
    /// Represents a 2D interger position.
    /// </summary>
    public struct Point {
        public int x;   // x positon
        public int y;   // y position

        /// <summary>
        /// Initializes a new instance of the <see cref="Zippy.Point"/> struct.
        /// </summary>
        /// <param name="X">X.</param>
        /// <param name="Y">Y.</param>
        public Point(int X, int Y) {
            x = X;
            y = Y;
        }

        /// <summary>
        /// Determines whether the specified <see cref="System.Object"/> is equal to the current <see cref="Zippy.Point"/>.
        /// </summary>
        /// <param name="obj">The <see cref="System.Object"/> to compare with the current <see cref="Zippy.Point"/>.</param>
        /// <returns><c>true</c> if the specified <see cref="System.Object"/> is equal to the current <see cref="Zippy.Point"/>;
        /// otherwise, <c>false</c>.</returns>
        public override bool Equals(object obj) {
            if (obj == null) {
                return false;
            }

            if (obj.GetType() != this.GetType()) {
                return false;
            }

            Point other = (Point)obj;
            return this.x == other.x && this.y == other.y;
        }

        /// <summary>
        /// Serves as a hash function for a <see cref="Zippy.Point"/> object.
        /// </summary>
        /// <returns>A hash code for this instance that is suitable for use in hashing algorithms and data structures such as a
        /// hash table.</returns>
        public override int GetHashCode() {
            return ToString().GetHashCode();
        }

        /// <summary>
        /// Returns a <see cref="System.String"/> that represents the current <see cref="Zippy.Point"/>.
        /// </summary>
        /// <returns>A <see cref="System.String"/> that represents the current <see cref="Zippy.Point"/>.</returns>
        public override string ToString() {
            return "(" + x + ", " + y + ")";
        }

        /// <summary>
        /// A point initialized to 0,0
        /// </summary>
        /// <value>The zero.</value>
        public static Point Zero {
            get {
                return new Point(0, 0);
            }
        }
    }
}
