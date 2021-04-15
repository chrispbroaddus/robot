using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

namespace Zippy.Interop {
    [StructLayout(LayoutKind.Sequential)]
    public struct SimulatorResetCommand {
        public int robot;
        public int world;
        public int streams;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct CameraConfigurationCommand {
        public CameraId cameraId;
        public int enable;
        public int enableDepth;
        public int enablePointcloud;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct StatsCommand {
        public int enable;
    }

    public delegate void ResetDelegate(SimulatorResetCommand reset);
    public delegate void CameraConfigurationDelegate(CameraConfigurationCommand cameraConfig);
    public delegate void StatsDelegate(StatsCommand stats);

    [StructLayout(LayoutKind.Sequential)]
    public struct SimulatorNetworkSettings {
        public int zmqSendRecvTimeoutMs;
        public int zmqLingerTimeMs;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct SimulatorCommandServerSettings {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string serverAddress;
        public CameraConfigurationDelegate cameraConfigCallback;
        public ResetDelegate resetCallback;
        public StatsDelegate statsCallback;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct SimulatorCameraPublisherSettings {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string frontLeftStereoAddress;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string frontLeftStereoDepthAddress;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string frontLeftStereoPointcloudAddress;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string frontRightStereoAddress;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string frontRightStereoDepthAddress;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string frontRightStereoPointcloudAddress;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string rearLeftStereoAddress;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string rearLeftStereoDepthAddress;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string rearLeftStereoPointcloudAddress;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string rearRightStereoAddress;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string rearRightStereoDepthAddress;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string rearRightStereoPointcloudAddress;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string frontFisheyeAddress;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string frontFisheyeDepthAddress;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string frontFisheyePointcloudAddress;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string rearFisheyeAddress;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string rearFisheyeDepthAddress;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string rearFisheyePointcloudAddress;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string leftFisheyeAddress;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string leftFisheyeDepthAddress;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string leftFisheyePointcloudAddress;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string rightFisheyeAddress;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string rightFisheyeDepthAddress;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string rightFisheyePointcloudAddress;

        public int zmqHighWaterMarkValue;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct SimulatorStatsPublisherSettings {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string statsAddress;
        public int zmqHighWaterMarkValue;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SimulatorNetworkManagerSettings {
        public SimulatorNetworkSettings network;
        public SimulatorCommandServerSettings commandServer;
        public SimulatorCameraPublisherSettings cameraPublisher;
        public SimulatorStatsPublisherSettings statsPublisher;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SimulatorStats {
        public float deltaTimeMs;
        public int frameNumber;
    }
}