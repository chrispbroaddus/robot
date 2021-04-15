using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Linq;

namespace Zippy {
    /// <summary>
    /// Image saver.
    /// Scene singleton instance used to control image saving
    /// </summary>
    public class ImageSaver : SingletonBase<ImageSaver> {
        bool _writeToDisk = false;

        void Start() {
            //Parse command line arguments
            var args = System.Environment.GetCommandLineArgs();

            if (args.Contains("-saveimages")) {
                Debug.Log("Saving Images");
                EnableWriteToDisk = true;
            }

            if (args.Contains("-savedir")) {
                var index = System.Array.IndexOf(args, "-savedir");
                index++;

                if (index < args.Length) {
                    ImageSaverWrapper.SetSaveLocation(args[index]);
                    Debug.Log("Saving Images to " + args[index]);
                }
            }
        }

        /// <summary>
        /// Toggle writing to disk, and get the current state
        /// </summary>
        /// <value><c>true</c> if enable write to disk; otherwise, <c>false</c>.</value>
        public bool EnableWriteToDisk {
            get {
                return _writeToDisk;
            }
            set {
                _writeToDisk = value;

                if (_writeToDisk) {
                    TextureDownloaderWrapper.AddImageProcessingCallback(ImageSaverWrapper.GetSaveImageCallback());
                }
                else {
                    TextureDownloaderWrapper.RemoveImageProcessingCallback(ImageSaverWrapper.GetSaveImageCallback());
                }
            }
        }
    }
}