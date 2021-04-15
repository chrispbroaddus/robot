using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;
using System;

namespace Zippy.Test {
    [RequireComponent(typeof(SceneLoader))]
    public class TestSceneLoader : MonoBehaviour {

        [Tooltip("URL or local path")]
        public string sceneBundleURL;

        // Use this for initialization
        void Start () {
            if (string.IsNullOrEmpty(sceneBundleURL)) {
                Debug.LogError ("No scene specified");
                return;
            }

            if (!IsAbsoluteUrl(sceneBundleURL)) {
                sceneBundleURL = "file://" + Path.GetFullPath (sceneBundleURL);
            }

            var loader = GetComponent<SceneLoader>();
            loader.Load(sceneBundleURL);
        }

        bool IsAbsoluteUrl(string url) {
            Uri result;
            return Uri.TryCreate(url, UriKind.Absolute, out result);
        }
    }
}