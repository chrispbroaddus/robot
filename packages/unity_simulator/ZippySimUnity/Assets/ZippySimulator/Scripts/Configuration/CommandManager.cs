using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;
using Zippy.Interop;

namespace Zippy {
    public class CommandManager : SingletonBase<CommandManager> {
        public event System.Action OnResetWorld;
        public event System.Action OnResetRobot;
        public event System.Action OnResetStreams;

        public event System.Action<CameraId, bool> OnEnableCamera;
        public event System.Action<CameraId, bool> OnEnableDepthOutput;
        public event System.Action<CameraId, bool> OnEnableXYZOutput;

        public event System.Action<bool> OnEnableStats;

        // Use this for initialization
        void Start () {
            SimulatorNetworkManager.OnCameraConfigurationCommand += HandleCameraConfiguration;
            SimulatorNetworkManager.OnSimulatorResetCommand += HandleReset;
            SimulatorNetworkManager.OnStatsCommand += HandleStats;
        }

        void HandleReset(SimulatorResetCommand reset) {
            Debug.LogFormat ("Reset Command: r {0}, w {1}, s {2}", reset.robot, reset.world, reset.streams);

            if (reset.world > 0 && OnResetWorld != null) {
                OnResetWorld ();
            }

            if (reset.robot > 0 && OnResetRobot != null) {
                OnResetRobot ();
            }

            if (reset.streams > 0 && OnResetStreams != null) {
                OnResetStreams ();
            }
        }

        void HandleCameraConfiguration(CameraConfigurationCommand camConfig) {
            Debug.LogFormat ("Enable Camera Command: {0}, enable {1}, depth {2}, xyz {3}", camConfig.cameraId, camConfig.enable, camConfig.enableDepth, camConfig.enablePointcloud);

            if (OnEnableCamera != null) {
                OnEnableCamera (camConfig.cameraId, camConfig.enable > 0);
            }

            if (OnEnableDepthOutput != null) {
                OnEnableDepthOutput (camConfig.cameraId, camConfig.enableDepth > 0);
            }

            if (OnEnableXYZOutput != null) {
                OnEnableXYZOutput (camConfig.cameraId, camConfig.enablePointcloud > 0);
            }
        }

        void HandleStats(StatsCommand stats) {
            if (OnEnableStats != null) {
                bool enable = stats.enable > 0;
                OnEnableStats (enable);
            }
        }
    }
}