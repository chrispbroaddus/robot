using UnityEngine;
using UnityEngine.Events;
using System.Collections;
using System.Collections.Generic;

namespace Zippy {
    public class ZippyEventManager : SingletonBase<ZippyEventManager> {

        Dictionary <ZippyEventType, ZippyEvent> m_eventDictionary = new Dictionary<ZippyEventType, ZippyEvent>();

        public static void StartListening (ZippyEventType zippyEvent, UnityAction<ZippyEventType, ZippyEventPayloadBase> listener) {
            if (IsDestroyed) {
                return;
            }

            ZippyEvent thisEvent = null;

            if (!Instance.m_eventDictionary.TryGetValue (zippyEvent, out thisEvent)) {
                thisEvent = new ZippyEvent ();
                Instance.m_eventDictionary.Add (zippyEvent, thisEvent);
            }

            thisEvent.AddListener (listener);
        }

        public static void StopListening (ZippyEventType zippyEvent, UnityAction<ZippyEventType, ZippyEventPayloadBase> listener) {
            if (IsDestroyed) {
                return;
            }

            ZippyEvent thisEvent = null;

            if (Instance.m_eventDictionary.TryGetValue (zippyEvent, out thisEvent)) {
                thisEvent.RemoveListener (listener);
            }
        }

        public static void SendEvent (ZippyEventType zippyEvent) {
            SendEvent (zippyEvent, null);
        }

        public static void SendEvent (ZippyEventType zippyEvent, ZippyEventPayloadBase payload) {
            if (IsDestroyed) {
                return;
            }

            ZippyEvent thisEvent = null;

            if (Instance.m_eventDictionary.TryGetValue (zippyEvent, out thisEvent)) {
                thisEvent.Invoke (zippyEvent, payload);
            }
        }

        public static void SendEvent<T> (ZippyEventType zippyEvent, T data) {
            ZippyEventPayloadBase payload = new ZippyEventPayload<T>(data);
            SendEvent (zippyEvent, payload);
        }
    }


}
