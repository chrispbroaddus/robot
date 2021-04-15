using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CachedTransform {
    public Vector3 localPosition {
        get;
        set;
    }
    public Quaternion localRotation {
        get;
        set;
    }
    public Vector3 localScale {
        get;
        set;
    }

    public CachedTransform() {
    }

    public CachedTransform(Transform t) {
        Save (t);
    }

    public void Save(Transform t) {
        localPosition = t.localPosition;
        localRotation = t.localRotation;
        localScale = t.localScale;
    }

    public void Apply(Transform t) {
        t.localPosition = localPosition;
        t.localRotation = localRotation;
        t.localScale = localScale;
    }
}
