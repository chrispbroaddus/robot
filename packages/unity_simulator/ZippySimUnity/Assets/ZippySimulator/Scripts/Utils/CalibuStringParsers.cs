using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Zippy {
    /// <summary>
    /// Calibu string parsers.
    /// Helper class for parsing string from calibu calibration files
    /// </summary>
    public static class CalibuStringParsers {
        /// <summary>
        /// Determines if the string is an array
        /// </summary>
        /// <returns><c>true</c> if is an array; otherwise, <c>false</c>.</returns>
        /// <param name="input">Input.</param>
        static bool IsArray(string input) {
            return input.StartsWith("[") && input.EndsWith("]");
        }

        /// <summary>
        /// Create an array of strings from the supplied string
        /// </summary>
        /// <returns>The array of strings.</returns>
        /// <param name="input">Input.</param>
        public static string[] StringArrayFromString(string input) {
            input = input.Trim();

            if (!IsArray(input)) {
                return null;
            }

            return input.Substring(1, input.Length - 2).Trim().Split(new char[] { ';' }, System.StringSplitOptions.RemoveEmptyEntries);
        }

        /// <summary>
        /// Create an array of floats from the supplied string
        /// </summary>
        /// <returns>The array of floats.</returns>
        /// <param name="input">Input.</param>
        public static float[] FloatArrayFromString(string input) {
            var stringArray = StringArrayFromString(input);

            if (stringArray == null) {
                return null;
            }

            var floatArray = new float[stringArray.Length];

            for (int ii = 0; ii < floatArray.Length; ii++) {
                floatArray[ii] = float.Parse(stringArray[ii]);
            }

            return floatArray;
        }

        /// <summary>
        /// Create a matrix from the supplied string
        /// </summary>
        /// <returns>The matrix from the string.</returns>
        /// <param name="input">Input. Input string</param>
        /// <param name="rows">Rows. Number of rows</param>
        /// <param name="cols">Cols. Number of columns</param>
        public static float[,] FloatMatrixFromString(string input, int rows, int cols) {
            var stringArray = StringArrayFromString(input);

            if (stringArray == null) {
                return null;
            }

            if (stringArray.Length != rows) {
                return null;
            }

            var matrix = new float[rows, cols];

            for (int rr = 0; rr < rows; rr++) {
                var row = stringArray[rr];
                var colElements = row.Split(new char[] { ',' }, System.StringSplitOptions.RemoveEmptyEntries);

                if (colElements.Length != cols) {
                    return null;
                }

                for (int cc = 0; cc < cols; cc++) {
                    matrix[rr, cc] = float.Parse(colElements[cc]);
                }
            }

            return matrix;
        }
    }
}
