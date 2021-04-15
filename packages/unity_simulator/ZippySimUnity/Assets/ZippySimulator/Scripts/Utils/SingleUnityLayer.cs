using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// Single unity layer.
/// Helper class to allow easy setting of a layer and a mask that uses that layer
/// </summary>
[System.Serializable]
public class SingleUnityLayer {
    [SerializeField]
    private int _layerIndex = 0;

    /// <summary>
    /// Gets or sets the index of the layer.
    /// </summary>
    /// <value>The index of the layer.</value>
    public int LayerIndex {
        get {
            return _layerIndex;
        }
        set {
            if (value >= 0 && value < 32) {
                _layerIndex = value;
            }
        }
    }

    /// <summary>
    /// Gets the mask for the layer.
    /// </summary>
    /// <value>The mask.</value>
    public int Mask {
        get {
            return 1 << _layerIndex;
        }
    }
}