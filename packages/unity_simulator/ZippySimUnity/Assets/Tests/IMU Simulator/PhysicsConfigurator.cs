using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PhysicsConfigurator : MonoBehaviour {

    public float fixedDeltaHz = 50;

    // Use this for initialization
    void Awake () {
        Time.fixedDeltaTime = 1.0f / fixedDeltaHz;
    }
}
