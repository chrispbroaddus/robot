using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Timers;
using UnityEngine;
using UnityEngine.UI;

namespace Zippy {
    [RequireComponent(typeof(Text))]
    public class VehicleVelocityText : MonoBehaviour {
        Text _text;

        void OnEnable () {
            _text = GetComponent<Text>();
            ZippyEventManager.StartListening (ZippyEventType.Velocity, HandleVelocityEvent);
        }

        void OnDisable () {
            ZippyEventManager.StopListening (ZippyEventType.Velocity, HandleVelocityEvent);
        }

        void HandleVelocityEvent(ZippyEventType zippyEvent, ZippyEventPayloadBase payload) {
            var p = payload as ZippyEventPayload<Vector3>;

            if (p != null) {
                _text.text = p.value + " m/s";
            }
        }
    }
}