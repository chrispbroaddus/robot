using UnityEngine;
using System;
using System.Runtime.InteropServices;

namespace Zippy {
    [StructLayout(LayoutKind.Sequential)]
    public struct FrameData {
        public CameraId cameraId;
        public ImageType imageType;
        public IntPtr texturePtr;
        public int width;
        public int height;
        public int frameNumber;
        public float timestamp;

        public FrameData(CameraId cameraId, ImageType imageType, Texture texture, int frameNumber, float timestamp) {
            if (texture == null) {
                throw new System.ArgumentNullException("texture cannot be null");
            }

            this.cameraId = cameraId;
            this.imageType = imageType;
            texturePtr = texture.GetNativeTexturePtr();
            width = texture.width;
            height = texture.height;
            this.frameNumber = frameNumber;
            this.timestamp = timestamp;
        }
    }
}