#if (UNITY_EDITOR_LINUX || UNITY_STANDALONE_LINUX) && !(UNITY_EDITOR_OSX || UNITY_EDITOR_WIN)
#define IMAGE_SAVER_SUPPORTED
#endif


    using System.Runtime.InteropServices;
using UnityEngine;

namespace Zippy {
    public static class ImageSaverWrapper {
        public static System.IntPtr GetSaveImageCallback() {
#if IMAGE_SAVER_SUPPORTED
            return ImageSaver_GetSaveImageFuncPtr();
#else
            return System.IntPtr.Zero;
#endif
        }

        public static void SetSaveLocation(string  location) {
#if IMAGE_SAVER_SUPPORTED
            ImageSaver_SetSaveLocation(location);
#endif
        }

#if IMAGE_SAVER_SUPPORTED
        [DllImport("simulated_zippy")]
        private static extern System.IntPtr ImageSaver_GetSaveImageFuncPtr();

        [DllImport("simulated_zippy", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern void ImageSaver_SetSaveLocation(string location);
#endif
    }
}
