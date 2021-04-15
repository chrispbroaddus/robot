using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

namespace Zippy {
    [RequireComponent(typeof(Toggle))]
    public class SendToggleEvent : MonoBehaviour {

        public ZippyEventType zippyEvent;

        Toggle m_toggle;

        // Use this for initialization
        void Start () {
            m_toggle = GetComponent<Toggle> ();
            m_toggle.onValueChanged.AddListener (HandleToggle);
        }

        void HandleToggle(bool isOn) {
            ZippyEventManager.SendEvent (zippyEvent, isOn);
        }
    }
}