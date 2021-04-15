using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

namespace Zippy {
    [RequireComponent(typeof(Text))]
    public class VideoServerStatus : MonoBehaviour {
        Text _text;

        // Use this for initialization
        void OnEnable () {
            _text = GetComponent<Text>();
            ZippyEventManager.StartListening (ZippyEventType.EnabledCameras, HandleEnabledCameras);
        }

        void OnDisable() {
            ZippyEventManager.StopListening (ZippyEventType.EnabledCameras, HandleEnabledCameras);
        }

        void HandleEnabledCameras(ZippyEventType zippyEvent, ZippyEventPayloadBase payload) {
            var p = payload as ZippyEventPayload<CameraId[]>;

            if (p == null) {
                return;
            }

            string myStr = "";

            foreach (var cameraId in p.value) {
                myStr += cameraId + ",";
            }

            _text.text = myStr;
        }
    }
}


