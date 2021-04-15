using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Zippy;

public class IMUReceiver : MonoBehaviour {
    IMUReading[] _readings;

    // Use this for initialization
    void Start () {
        ZippyEventManager.StartListening (ZippyEventType.IMUData, HandleImuReadings);
    }

    void OnDestroy() {
        ZippyEventManager.StopListening (ZippyEventType.IMUData, HandleImuReadings);
    }

    void HandleImuReadings(ZippyEventType zippyEvent, ZippyEventPayloadBase payload) {
        _readings = (payload as ZippyEventPayload<IMUReading[]>).value;
    }

    void OnGUI() {
        if (_readings == null || _readings.Length <= 0) {
            return;
        }

        GUILayout.Label("Readings: " + _readings.Length);
        GUILayout.Label("Gyro: " + _readings[_readings.Length - 1].gyro);
        GUILayout.Label("Acc:  " + _readings[_readings.Length - 1].accel);
        var hz = 1.0f / Time.fixedDeltaTime;
        GUILayout.Label ("Fixed Delta Time: " + hz + "Hz");
    }
}
