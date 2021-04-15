using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Linq;

namespace Zippy {
    /// <summary>
    /// Image streamer.
    /// Scene singleton instance used to control image streaming
    /// </summary>
    public class ImageStreamer : SingletonBase<ImageStreamer> {
        bool _streamToNetwork = false;

        /// <summary>
        /// Toggle streaming to network, and get the current state
        /// </summary>
        /// <value><c>true</c> if enable stream to network; otherwise, <c>false</c>.</value>
        public bool EnableStreamToNetwork {
            get {
                return _streamToNetwork;
            }
            set {
                _streamToNetwork = value;

                if (_streamToNetwork) {
                    TextureDownloaderWrapper.AddImageProcessingCallback(SimulatorNetworkManager.SendImageCallback());
                }
                else {
                    TextureDownloaderWrapper.RemoveImageProcessingCallback(SimulatorNetworkManager.SendImageCallback());
                }
            }
        }
    }
}