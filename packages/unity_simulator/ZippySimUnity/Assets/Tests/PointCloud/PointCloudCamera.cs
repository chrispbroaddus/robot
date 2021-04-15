using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Zippy;

//TEST CAMERA. DO NOT USE IN SIMULATOR
[RequireComponent(typeof(Camera))]
public class PointCloudCamera : MonoBehaviour {
    public UnityEngine.UI.RawImage uiImage;
    public bool enableDownload = false;

    Camera _camera;
    Shader _shader;
    Material _material;
    RenderTexture _renderTexture;

    void Awake () {
        if (!SetUpImageEffect()) {
            return;
        }

        //configure the camera
        _camera = GetComponent<Camera>();
        _camera.depthTextureMode = DepthTextureMode.Depth;
        _renderTexture = new RenderTexture(Screen.width, Screen.height, 24, RenderTextureFormat.ARGBFloat);
        _renderTexture.filterMode = FilterMode.Bilinear;
        _renderTexture.wrapMode = TextureWrapMode.Clamp;
        _renderTexture.useMipMap = false;
        _camera.targetTexture = _renderTexture;
        uiImage.texture = _renderTexture;
        var projectionMatrix = GL.GetGPUProjectionMatrix (_camera.projectionMatrix, true);
        _material.SetMatrix ("projInv", projectionMatrix.inverse);
        ImageSaver.Instance.EnableWriteToDisk = true;
        //start the download update coroutine
        StartCoroutine(UpdateDownloads());
    }

    void OnDestroy() {
        if (_material) {
            DestroyImmediate(_material);
        }

        _material = null;
        _renderTexture = null;
    }


    bool SetUpImageEffect() {
        // Disable if we don't support image effects
        if (!SystemInfo.supportsImageEffects) {
            Debug.LogError("System does not support image effects. CameraRig disabled");
            enabled = false;
            return false;
        }

        _shader = Shader.Find("Zippy/PointCloud");

        // Disable the image effect if the shader can't run on the users graphics card
        if (!_shader || !_shader.isSupported) {
            Debug.LogError("PointCloud Shader missing or not supported");
            enabled = false;
            return false;
        }

        _material = new Material(_shader);
        _material.hideFlags = HideFlags.HideAndDontSave;
        return true;
    }

    void OnRenderImage(RenderTexture source, RenderTexture destination) {
        Graphics.Blit(source, destination, _material);
        Texture2D texture = null;

        if (enableDownload) {
            if (texture != null) {
                StartDownload (_renderTexture, ImageType.XYZ);
            }
        }
    }

    void StartDownload(Texture tex, ImageType imageType) {
        var frameData = new FrameData(CameraId.FrontFisheye, imageType, tex, Time.frameCount, Time.unscaledTime);
        var status = TextureDownloaderWrapper.StartDownload(frameData);

        if (TextureDownloaderWrapper.Failed(status)) {
            Debug.LogError(CameraId.FrontFisheye + " failed to start downloading " + imageType + "texture: " + status);
        }
    }

    /// <summary>
    /// Coroutine to update the downloads every frame
    /// </summary>
    /// <returns>The downloads.</returns>
    IEnumerator UpdateDownloads() {
        while (true) {
            yield return new WaitForEndOfFrame();
            TextureDownloaderWrapper.UpdateDownloads();
        }
    }
}
