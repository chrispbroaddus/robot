using UnityEngine;

namespace Zippy {
    /// <summary>
    /// Singleton base.
    /// Used to make singleton Monobehaviours
    /// </summary>
    public abstract class SingletonBase<T> : MonoBehaviour where T : UnityEngine.Component {
        static T _instance = null;
        static bool _destroyed = false;

        /// <summary>
        /// Gets the instance if it exists. Does not create a new instance
        /// </summary>
        /// <value>The instance.</value>
        public static T Instance {
            get {
                if (_destroyed) {
                    return null;
                }

                if (_instance == null) {
                    _instance = FindObjectOfType<T> ();

                    if (_instance == null) {
                        throw new System.Exception("Instance of " + typeof(T).Name + " not found in scene.");
                    }
                }

                return _instance;
            }
        }

        protected virtual void Awake () {
            if (_instance == null) {
                _instance = this as T;
                _destroyed = false;
            }
            else if (_instance != this as T) {
                Debug.LogError ("Instance of " + typeof(T).Name + " already exists. Deleting " + name);
                Destroy (gameObject);
            }
            else {
                _destroyed = false;
            }
        }

        protected virtual void OnDestroy () {
            if (this == _instance) {
                _destroyed = true;
                _instance = null;
            }
        }

        protected static bool IsDestroyed {
            get {
                return _destroyed;
            }
        }
    }
}