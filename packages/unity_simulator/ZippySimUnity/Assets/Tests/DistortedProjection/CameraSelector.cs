using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Zippy;

[RequireComponent(typeof(CameraRigViewerBase))]
public class CameraSelector : MonoBehaviour {
    CameraRigViewerBase _viewer;
    CameraRig _rig;

    // Use this for initialization
    void Start () {
        _viewer = GetComponent<CameraRigViewerBase> ();
        _rig = FindObjectOfType<CameraRig> ();
        _viewer.SelectedCamera = _rig.cameraId;
    }
}
