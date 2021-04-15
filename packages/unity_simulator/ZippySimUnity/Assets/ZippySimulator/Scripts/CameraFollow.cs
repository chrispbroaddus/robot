using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CameraFollow : MonoBehaviour {

    public float speed = 2.0f;

    Transform _targetTransform;
    Transform _transform;
    // Use this for initialization
    void Start () {
        _transform = transform;
    }

    // Update is called once per frame
    void FixedUpdate () {
        if (_targetTransform == null) {
            var target = FindObjectOfType<CameraFollowTarget> ();

            if (target != null) {
                _targetTransform = target.transform;
            }
        }
        else {
            _transform.position = Vector3.Lerp (_transform.position, _targetTransform.position, Time.deltaTime * speed);
            _transform.rotation = Quaternion.Slerp (_transform.rotation, _targetTransform.rotation, Time.deltaTime * speed);
        }
    }
}
