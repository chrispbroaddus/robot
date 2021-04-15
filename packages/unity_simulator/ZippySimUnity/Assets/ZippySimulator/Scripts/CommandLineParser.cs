using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Zippy {
    public static class CommandLineParser {
        public static string GetArgValue(string argName) {
            if (string.IsNullOrEmpty (argName)) {
                return string.Empty;
            }

            var args = System.Environment.GetCommandLineArgs();

            for (int ii = 0; ii < args.Length; ++ii) {
                if (args[ii] == argName) {
                    if (ii + 1 < args.Length) {
                        return args[ii + 1];
                    }
                }
            }

            return string.Empty;
        }

        public static bool ArgExists(string argName) {
            if (string.IsNullOrEmpty (argName)) {
                return false;
            }

            var args = System.Environment.GetCommandLineArgs();

            for (int ii = 0; ii < args.Length; ii++) {
                if (args[ii] == argName) {
                    return true;
                }
            }

            return false;
        }
    }
}