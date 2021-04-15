using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;
using Zippy.Interop;

namespace Zippy {
    public class SimulatorNetworkManager : SingletonBase<SimulatorNetworkManager> {
        public static event System.Action<CameraConfigurationCommand> OnCameraConfigurationCommand;
        public static event System.Action<SimulatorResetCommand> OnSimulatorResetCommand;
        public static event System.Action<StatsCommand> OnStatsCommand;

        [DllImport("simulated_zippy", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern void SimulatorNetworkManager_Initialize (SimulatorNetworkManagerSettings settings);

        [DllImport("simulated_zippy")]
        private static extern void SimulatorNetworkManager_Shutdown ();

        [DllImport("simulated_zippy")]
        private static extern void SimulatorNetworkManager_Update ();

        [DllImport("simulated_zippy")]
        private static extern System.IntPtr SimulatorNetworkManager_GetSendImageFuncPtr ();

        [DllImport("simulated_zippy", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern void SimulatorNetworkManager_SendStats (SimulatorStats stats);

        void Start() {
            SimulatorNetworkManagerSettings settings = new SimulatorNetworkManagerSettings();
            settings.network.zmqLingerTimeMs = SimulatorSettingsManager.Settings.networking.zmqLingerTimeMs;
            settings.network.zmqSendRecvTimeoutMs = SimulatorSettingsManager.Settings.networking.zmqSendReceiveTimeoutMs;
            settings.commandServer.serverAddress = ConnectionsReader.FullAddress (ConnectionTopic.simulator_commands);
            settings.commandServer.cameraConfigCallback = HandleCameraConfiguration;
            settings.commandServer.resetCallback = HandleReset;
            settings.commandServer.statsCallback = HandleStats;
            settings.cameraPublisher.frontLeftStereoAddress = ConnectionsReader.FullAddress (ConnectionTopic.front_left_stereo);
            settings.cameraPublisher.frontLeftStereoDepthAddress = ConnectionsReader.FullAddress (ConnectionTopic.front_left_stereo_depth);
            settings.cameraPublisher.frontLeftStereoPointcloudAddress = ConnectionsReader.FullAddress (ConnectionTopic.front_left_stereo_pointcloud);
            settings.cameraPublisher.frontFisheyeAddress = ConnectionsReader.FullAddress (ConnectionTopic.front_fisheye);
            settings.cameraPublisher.frontFisheyeDepthAddress = ConnectionsReader.FullAddress (ConnectionTopic.front_fisheye_depth);
            settings.cameraPublisher.frontFisheyePointcloudAddress = ConnectionsReader.FullAddress (ConnectionTopic.front_fisheye_pointcloud);
            settings.cameraPublisher.frontRightStereoAddress = ConnectionsReader.FullAddress (ConnectionTopic.front_right_stereo);
            settings.cameraPublisher.frontRightStereoDepthAddress = ConnectionsReader.FullAddress (ConnectionTopic.front_right_stereo_depth);
            settings.cameraPublisher.frontRightStereoPointcloudAddress = ConnectionsReader.FullAddress (ConnectionTopic.front_right_stereo_pointcloud);
            settings.cameraPublisher.leftFisheyeAddress = ConnectionsReader.FullAddress (ConnectionTopic.left_fisheye);
            settings.cameraPublisher.leftFisheyeDepthAddress = ConnectionsReader.FullAddress (ConnectionTopic.left_fisheye_depth);
            settings.cameraPublisher.leftFisheyePointcloudAddress = ConnectionsReader.FullAddress (ConnectionTopic.left_fisheye_pointcloud);
            settings.cameraPublisher.rearFisheyeAddress = ConnectionsReader.FullAddress (ConnectionTopic.rear_fisheye);
            settings.cameraPublisher.rearFisheyeDepthAddress = ConnectionsReader.FullAddress (ConnectionTopic.rear_fisheye_depth);
            settings.cameraPublisher.rearFisheyePointcloudAddress = ConnectionsReader.FullAddress (ConnectionTopic.rear_fisheye_pointcloud);
            settings.cameraPublisher.rearLeftStereoAddress = ConnectionsReader.FullAddress (ConnectionTopic.rear_left_stereo);
            settings.cameraPublisher.rearLeftStereoDepthAddress = ConnectionsReader.FullAddress (ConnectionTopic.rear_left_stereo_depth);
            settings.cameraPublisher.rearLeftStereoPointcloudAddress = ConnectionsReader.FullAddress (ConnectionTopic.rear_left_stereo_pointcloud);
            settings.cameraPublisher.rearRightStereoAddress = ConnectionsReader.FullAddress (ConnectionTopic.rear_right_stereo);
            settings.cameraPublisher.rearRightStereoDepthAddress = ConnectionsReader.FullAddress (ConnectionTopic.rear_right_stereo_depth);
            settings.cameraPublisher.rearRightStereoPointcloudAddress = ConnectionsReader.FullAddress (ConnectionTopic.rear_right_stereo_pointcloud);
            settings.cameraPublisher.rightFisheyeAddress = ConnectionsReader.FullAddress (ConnectionTopic.right_fisheye);
            settings.cameraPublisher.rightFisheyeDepthAddress = ConnectionsReader.FullAddress (ConnectionTopic.right_fisheye_depth);
            settings.cameraPublisher.rightFisheyePointcloudAddress = ConnectionsReader.FullAddress (ConnectionTopic.right_fisheye_pointcloud);
            settings.cameraPublisher.zmqHighWaterMarkValue = SimulatorSettingsManager.Settings.networking.zmqHighWaterMark;
            settings.statsPublisher.statsAddress = ConnectionsReader.FullAddress(ConnectionTopic.simulator_stats);
            settings.statsPublisher.zmqHighWaterMarkValue = SimulatorSettingsManager.Settings.networking.zmqHighWaterMark;
            SimulatorNetworkManager_Initialize (settings);
        }

        void Update () {
            SimulatorNetworkManager_Update ();
        }

        protected override void OnDestroy() {
            SimulatorNetworkManager_Shutdown ();
            base.OnDestroy ();
        }

        static public System.IntPtr SendImageCallback() {
            return SimulatorNetworkManager_GetSendImageFuncPtr ();
        }

        static public void SendStats(SimulatorStats stats) {
            SimulatorNetworkManager_SendStats (stats);
        }

        [AOT.MonoPInvokeCallback (typeof(ResetDelegate))]
        static void HandleReset(SimulatorResetCommand reset) {
            if (OnSimulatorResetCommand != null) {
                OnSimulatorResetCommand (reset);
            }
        }

        [AOT.MonoPInvokeCallback (typeof(CameraConfigurationDelegate))]
        static void HandleCameraConfiguration(CameraConfigurationCommand camConfig) {
            if (OnCameraConfigurationCommand != null) {
                OnCameraConfigurationCommand (camConfig);
            }
        }


        [AOT.MonoPInvokeCallback (typeof(StatsDelegate))]
        static void HandleStats(StatsCommand stats) {
            if (OnStatsCommand != null) {
                OnStatsCommand (stats);
            }
        }

    }
}
