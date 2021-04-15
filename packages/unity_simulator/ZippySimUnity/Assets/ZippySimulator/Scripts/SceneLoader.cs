using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Zippy {
    public class SceneLoader : MonoBehaviour {
        public event System.Action<Status> OnLoadStatusChanged;
        public event System.Action<float> OnLoadProgressChanged;

        public enum Status {
            Unloaded,
            Loading,
            Loaded,
            Failed,
        };

        public Status LoadStatus {
            get;
            private set;
        }

        public GameObject[] SceneGameObjects {
            get;
            private set;
        }

        AssetBundle m_assetBundle;
        Coroutine m_runningLoader;
        WWW m_www;

        public bool Load(string url) {
            if (LoadStatus == Status.Loaded || LoadStatus == Status.Loading) {
                return false;
            }

            m_runningLoader = StartCoroutine(Loader(url));
            return true;
        }

        public void Unload() {
            if (m_runningLoader != null) {
                StopCoroutine(m_runningLoader);
            }

            if (m_www != null) {
                m_www = null;
            }

            if (SceneGameObjects != null) {
                foreach (var go in SceneGameObjects) {
                    Destroy(go);
                }

                SceneGameObjects = null;
            }

            if (m_assetBundle != null) {
                m_assetBundle.Unload(true);
            }

            LoadStatus = Status.Unloaded;
            SendStatus();
        }

        IEnumerator Loader(string url) {
            LoadStatus = Status.Loading;
            m_www = new WWW(url);
            yield return m_www;

            if (!string.IsNullOrEmpty(m_www.error)) {
                Debug.LogError("Asset bundle could not be downloaded: " + m_www.error);
                LoadStatus = Status.Failed;
                SendStatus();
                yield break;
            }

            var ab = m_www.assetBundle;

            if (ab == null) {
                Debug.LogError("URL was not a valid asset bundle");
                LoadStatus = Status.Failed;
                SendStatus();
                yield break;
            }

            var gos = ab.LoadAllAssets<GameObject>();

            if (gos == null || gos.Length <= 0) {
                Debug.LogError("No Scene GameObjects in bundle");
                LoadStatus = Status.Failed;
                SendStatus();
                Unload();
                yield break;
            }

            var sceneGOs = new GameObject[gos.Length];

            for (int ii = 0; ii < gos.Length; ii++) {
                sceneGOs[ii] = Instantiate(gos[ii]);
            }

            SceneGameObjects = sceneGOs;
            LoadStatus = Status.Loaded;
            m_runningLoader = null;
            m_www = null;
            SendStatus();
        }

        void SendStatus() {
            if (OnLoadStatusChanged != null) {
                OnLoadStatusChanged(LoadStatus);
            }
        }

        void SendProgress() {
            if (OnLoadProgressChanged != null) {
                OnLoadProgressChanged(m_www.progress);
            }
        }

        void Update() {
            if (LoadStatus == Status.Loading && m_www != null) {
                SendProgress();
            }
        }
    }
}
