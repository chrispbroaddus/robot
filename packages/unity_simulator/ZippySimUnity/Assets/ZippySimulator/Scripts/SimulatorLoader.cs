using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

namespace Zippy {
    [RequireComponent(typeof(SceneLoader))]
    public class SimulatorLoader : MonoBehaviour {

        SceneLoader m_sceneLoader;

        IEnumerator Start() {
            GlogWrapper.Init ();
            m_sceneLoader = GetComponent<SceneLoader>();
            m_sceneLoader.OnLoadStatusChanged += HandleSceneLoadStatusChanged;
            LoadMain ();
            yield return null;
            LoadScene ();
        }

        void LoadMain () {
            SceneManager.LoadScene ("Main", LoadSceneMode.Additive);
        }

        void HandleSceneLoadStatusChanged (SceneLoader.Status status) {
            if (status == SceneLoader.Status.Loaded) {
                StartCoroutine(PostSceneLoad ());
            }
            else if (status == SceneLoader.Status.Failed) {
                Application.Quit();
            }
        }

        void LoadScene() {
            //command linge scene overrides settings file
            //if no scene is specified on command line or settings file, load the fallback scene
            var sceneFile = CommandLineParser.GetArgValue("-scene");

            if (string.IsNullOrEmpty(sceneFile)) {
                sceneFile = SimulatorSettingsManager.Settings.scene.sceneUrl;
            }

            if (string.IsNullOrEmpty(sceneFile)) {
                SceneManager.LoadScene("Fallback", LoadSceneMode.Additive);
                StartCoroutine(PostSceneLoad ());
            }
            else if (!m_sceneLoader.Load(sceneFile)) {
                Application.Quit();
            }
        }

        void LoadRobot() {
            switch (SimulatorSettingsManager.Settings.vehicle.model) {
            case SimulatedVehicleModel.BoxCar:
                SceneManager.LoadScene ("ZippyBoxcar", LoadSceneMode.Additive);
                break;

            case SimulatedVehicleModel.ZippyFromCAD:
            default:
                SceneManager.LoadScene ("ZippyCad", LoadSceneMode.Additive);
                break;
            }
        }

        IEnumerator PostSceneLoad() {
            yield return null;
            LoadRobot();
            m_sceneLoader.OnLoadStatusChanged -= HandleSceneLoadStatusChanged;
            yield return null;
            Destroy (gameObject);
        }
    }

}
