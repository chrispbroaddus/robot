#if (UNITY_EDITOR_LINUX || UNITY_STANDALONE_LINUX) && !(UNITY_EDITOR_OSX || UNITY_EDITOR_WIN)
#define TEXTURE_DOWNLOADER_SUPPORTED
#endif

    using UnityEngine;
using System.Runtime.InteropServices;
using System;

namespace Zippy {
    public static class TextureDownloaderWrapper {
        public enum Status {
            Succeeded = 0,              // Operation succeeded
            InProgress,                 // Operation in progress
            Error_GlewInit,             // Glew failed to initialize
            Error_UnsupportedAPI,       // Grahpics API is not supported
            Error_InvalidArguments,     // Invalid arguments were supplied
            Error_TooManyRequests,      // Requesting too many downloads. Let other downloads finish first
            Error_DownloadNoData,       // Download completed, but got no data
            Error_DownloadWait,         // Waiting for the download failed
            Error_UnsupportedPlatform,  // Active platform is not supported
            Error_UninitializedQueue    // Trying to use a queue that has not been set up
        }

        /// <summary>
        /// Check if status is an error status
        /// </summary>
        /// <param name="status"></param>
        /// <returns></returns>
        public static bool Failed(Status status) {
            return !(status == Status.Succeeded || status == Status.InProgress);
        }

        static IntPtr _startDownloadFuncPtr;        // pointer to the start download function for the render thread
        static IntPtr _updateDownloadsFuncPtr;      // pointer to the update downloads function for the render thread
        static IntPtr _initializeCameraFuncPtr;     // pointer to initialize camera download queue function  for the render thread
        static IntPtr _cleanUpFuncPtr;              // pointer to cleanup download queues for the render thread

        /// <summary>
        /// Get the func ptr references
        /// </summary>
        static void GetFunctionPtrs() {
#if TEXTURE_DOWNLOADER_SUPPORTED
            _startDownloadFuncPtr = TextureDownloader_GetStartDownloadFunctionPtr();
            _updateDownloadsFuncPtr = TextureDownloader_GetUpdateDownloadsFunctionPtr();
            _cleanUpFuncPtr = TextureDownloader_GetCleanUpFunctionPtr();
#endif
        }

        /// <summary>
        /// Initializes the texture downloader. Call before making any other calls to the downloader
        /// </summary>
        /// <returns>The texture downloader.</returns>
        public static void InitializeTextureDownloader() {
#if TEXTURE_DOWNLOADER_SUPPORTED
            TextureDownloader_Initialize(SimulatorSettingsManager.Settings.cameras.frameDownloadBufferSize);
            GetFunctionPtrs();
#endif
        }

        /// <summary>
        /// Start downloading the specified frame
        /// </summary>
        /// <param name="frameData"></param>
        /// <returns></returns>
        public static Status StartDownload(FrameData frameData) {
#if TEXTURE_DOWNLOADER_SUPPORTED
            Status status = (Status)TextureDownloader_StartDownload(frameData);

            if (status == Status.Succeeded) {
                GL.IssuePluginEvent(_startDownloadFuncPtr, 0);
            }

#if UNITY_EDITOR // check for errors in editor

            if (Failed(status)) {
                Debug.LogError("StartDownload failed: " + status);
            }

#endif // UNITY_EDITOR
            return status;
#else
            return Status.Error_UnsupportedPlatform;
#endif
        }

        public static void CleanUp(CameraId cameraId) {
#if TEXTURE_DOWNLOADER_SUPPORTED
            TextureDownloader_CleanUpAll(cameraId);
            GL.IssuePluginEvent(_cleanUpFuncPtr, 0);
#endif
        }

        public static void CleanUp(CameraId cameraId, ImageType imageType) {
#if TEXTURE_DOWNLOADER_SUPPORTED
            TextureDownloader_CleanUp(cameraId, imageType);
            GL.IssuePluginEvent(_cleanUpFuncPtr, 0);
#endif
        }

        /// <summary>
        /// This has to be called every frame, otherwise the plugin won't keep checking the downloads
        /// </summary>
        public static void UpdateDownloads() {
#if TEXTURE_DOWNLOADER_SUPPORTED
            GL.IssuePluginEvent(_updateDownloadsFuncPtr, 0);
#endif
        }

        public static UnityEngine.Rendering.CommandBuffer UpdateDownloadsCommandBuffer () {
            var cb = new UnityEngine.Rendering.CommandBuffer ();
            cb.IssuePluginEvent (_updateDownloadsFuncPtr, 0);
            return cb;
        }

        /// <summary>
        /// Add a callback function that will receive the downloaded images
        /// Callbacks should conform to the ProcessImageFuncPtr defiend in the zippy_image_interop.h C++ header file
        /// </summary>
        /// <param name="processingCallback"></param>
        public static void AddImageProcessingCallback(IntPtr processingCallback) {
#if TEXTURE_DOWNLOADER_SUPPORTED
            TextureDownloaderWrapper.TextureDownloader_AddImageProcessingCallback(processingCallback);
#endif
        }

        /// <summary>
        /// Remove a callback function that receives the downloaded images
        /// Callbacks should conform to the ProcessImageFuncPtr defiend in the zippy_image_interop.h C++ header file
        /// </summary>
        /// <param name="processingCallback"></param>
        public static void RemoveImageProcessingCallback(IntPtr processingCallback) {
#if TEXTURE_DOWNLOADER_SUPPORTED
            TextureDownloaderWrapper.TextureDownloader_RemoveImageProcessingCallback(processingCallback);
#endif
        }

        #region DllImport
#if TEXTURE_DOWNLOADER_SUPPORTED
        [DllImport("simulated_zippy")]
        private static extern void TextureDownloader_Initialize(int downloadQueueSize);

        [DllImport("simulated_zippy")]
        private static extern int TextureDownloader_StartDownload(FrameData frameData);

        [DllImport("simulated_zippy")]
        private static extern void TextureDownloader_CleanUp (CameraId cameraId, ImageType imageType);

        [DllImport("simulated_zippy")]
        private static extern void TextureDownloader_CleanUpAll (CameraId cameraId);

        [DllImport("simulated_zippy")]
        private static extern IntPtr TextureDownloader_GetStartDownloadFunctionPtr();

        [DllImport("simulated_zippy")]
        private static extern IntPtr TextureDownloader_GetUpdateDownloadsFunctionPtr();

        [DllImport("simulated_zippy")]
        private static extern IntPtr TextureDownloader_GetCleanUpFunctionPtr();

        [DllImport("simulated_zippy")]
        private static extern int TextureDownloader_GetLastStatus();

        [DllImport("simulated_zippy")]
        private static extern void TextureDownloader_AddImageProcessingCallback(IntPtr functionPointer);

        [DllImport("simulated_zippy")]
        private static extern void TextureDownloader_RemoveImageProcessingCallback(IntPtr functionPointer);

#endif
        #endregion
    }
}
