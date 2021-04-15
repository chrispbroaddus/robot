using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Zippy;

public class CroppedFisheyeTest : MonoBehaviour {

    CameraRigManager m_cameraRigManager;
    CameraRigImageViewer m_imageViewer;

    // Use this for initialization
    void OnEnable () {
        m_cameraRigManager = FindObjectOfType<CameraRigManager> ();
        m_imageViewer = FindObjectOfType<CameraRigImageViewer> ();
        ZippyEventManager.StartListening (ZippyEventType.CamerasReady, HandleEvents);
        ZippyEventManager.StartListening (ZippyEventType.CameraEnabled, HandleEvents);
    }

    void HandleEvents(ZippyEventType type, ZippyEventPayloadBase payload) {
        switch (type) {
        case ZippyEventType.CamerasReady:
            m_imageViewer.SelectedCamera = CameraId.FrontFisheye;
            break;
        }
    }
}
