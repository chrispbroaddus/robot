using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

/// <summary>
/// A very simple FPS counter that renders to an attached Text UI element
/// </summary>
[RequireComponent(typeof(Text))]
public class FPSLabel : MonoBehaviour {
    Text _text;

    // Use this for initialization
    void Start () {
        _text = GetComponent<Text>();
    }

    // Update is called once per frame
    void Update () {
        var fps = 1.0f / Time.smoothDeltaTime;
        _text.text = fps.ToString("0.00") + " fps / " + Time.smoothDeltaTime.ToString("0.000") + " ms";
    }
}
