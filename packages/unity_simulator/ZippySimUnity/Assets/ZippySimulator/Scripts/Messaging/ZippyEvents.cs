using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Events;

namespace Zippy {
    public enum ZippyEventType {
        Velocity,                   //Vector3
        KeyboardControl,            //bool
        JoystickControl,            //bool
        TrajectoryControl,          //bool
        CamerasReady,               //null
        RobotReady,                 //null
        EnabledCameras,             //array of camera ids
        CameraEnabled,              //CameraOutputPayload
        CameraDepthEnabled,         //CameraOutputPayload
        CameraXYZEnabled,           //CameraOutputPayload
        IMUData,                    //IMUReading[]
        GPSData,                    //GlobalPosition
    }

    [System.Serializable]
    public abstract class ZippyEventPayloadBase {

    }

    public class ZippyEventPayload<T> : ZippyEventPayloadBase {
        public T value;

        public ZippyEventPayload() {}

        public ZippyEventPayload(T v) {
            value = v;
        }
    }

    /// <summary>
    /// Contains a camera id and a bool to indicate if the output is enabled or not.
    /// Which output is defined by the event type
    /// </summary>
    public class CameraOutputPayload : ZippyEventPayloadBase {
        public CameraId cameraId;
        public bool enabled;

        public CameraOutputPayload(CameraId id, bool outputEnabled) {
            cameraId = id;
            enabled = outputEnabled;
        }
    }

    [System.Serializable]
    public class ZippyEvent : UnityEvent<ZippyEventType, ZippyEventPayloadBase> {

    }
}
