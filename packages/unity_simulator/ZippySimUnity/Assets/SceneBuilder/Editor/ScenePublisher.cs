using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using System.IO;
using UnityEditor.SceneManagement;

namespace Zippy {
    public class ScenePublisher : Editor {
        static readonly string SCENE_PREFAB_DIR = "Assets/ScenePrefabs";
        static readonly string SCENE_BUNDLES_DIR = "Assets/SceneBundles";
        static readonly string EXTENSION = ".zss";

        [MenuItem("Zippy/Create Scene Root &r")]
        public static void CreateRoot() {
            var root = FindObjectOfType<SceneRoot>();

            if (root == null) {
                var go = new GameObject("Scene Root");
                root = go.AddComponent<SceneRoot>();
            }

            Selection.activeObject = root.gameObject;
        }

        [MenuItem("Zippy/Create Scene Root &r", true)]
        public static bool ValidateCreateRoot() {
            var root = FindObjectOfType<SceneRoot>();
            return root == null;
        }

        [MenuItem("Zippy/Publish Scene &p")]
        public static void MenuPublish() {
            var root = FindObjectOfType<SceneRoot>();

            if (root == null) {
                EditorUtility.DisplayDialog("Publish Error", "Unable to publish. No SceneRoot in scene.", "Oh.");
                return;
            }

            if (!EditorSceneManager.EnsureUntitledSceneHasBeenSaved("Save scene before publishing")) {
                EditorUtility.DisplayDialog("Publish Error", "Publishing aborted. Scene must be saved.", "OK");
                return;
            }

            if (!EditorSceneManager.SaveCurrentModifiedScenesIfUserWantsTo()) {
                EditorUtility.DisplayDialog("Publish Error", "Publishing aborted. Scene must be saved.", "OK");
                return;
            }

            string sceneName = SanitizeName(EditorSceneManager.GetActiveScene().name);

            if (string.IsNullOrEmpty(sceneName)) {
                EditorUtility.DisplayDialog("Publish Error", "Scene name is not valid. Please rename", "OK");
                return;
            }

            string path = Publish(root, sceneName, SCENE_BUNDLES_DIR);

            if (string.IsNullOrEmpty(path)) {
                EditorUtility.DisplayDialog("Publish Error", "Failed to create scene asset bundle", "OK");
            }
            else {
                EditorUtility.DisplayDialog("Publish Success", "Created scene asset bundle at " + path, "OK");
            }
        }

        [MenuItem("Zippy/Publish Scene &p", true)]
        public static bool ValidateMenuPublish() {
            var root = FindObjectOfType<SceneRoot>();
            return root != null;
        }

        static string SanitizeName(string name) {
            name = name.Replace(" ", "");
            //TODO strip all whitespace, and non ascii characters. leave only a-zA-Z0-9-_.
            return name;
        }

        public static string Publish(SceneRoot root, string sceneName, string outputDir) {
            //validate inputs
            if (root == null) {
                Debug.LogError("root cannot be null");
                return string.Empty;
            }

            if (string.IsNullOrEmpty(sceneName)) {
                Debug.LogError("sceneName cannot be null or empty");
                return string.Empty;
            }

            if (outputDir == null) {
                Debug.LogError("outputDir cannot be null");
                return string.Empty;
            }

            //make sure prefab dir exists
            if (!Directory.Exists(SCENE_PREFAB_DIR)) {
                Directory.CreateDirectory(SCENE_PREFAB_DIR);
            }

            //form filenames and paths
            string fileName = sceneName + EXTENSION;
            string fullPath = Path.Combine(outputDir, fileName);
            //save scene as a prefab
            string prefabName = Path.Combine(SCENE_PREFAB_DIR, sceneName + ".prefab");
            PrefabUtility.CreatePrefab(prefabName, root.gameObject);
            // Create the array of bundle build details.
            AssetBundleBuild[] buildMap = new AssetBundleBuild[1];
            buildMap[0].assetBundleName = fileName;
            string[] sceneAssets = new string[1];
            sceneAssets[0] = prefabName;
            buildMap[0].assetNames = sceneAssets;

            //create output dir if requried
            if (!Directory.Exists(outputDir)) {
                Directory.CreateDirectory(outputDir);
            }

            var manifest = BuildPipeline.BuildAssetBundles(outputDir, buildMap, BuildAssetBundleOptions.StrictMode, BuildTarget.StandaloneLinux64);

            if (manifest == null) {
                Debug.LogError("Failed to create bundle: " + fullPath);
                return string.Empty;
            }

            return fullPath;
        }
    }
}