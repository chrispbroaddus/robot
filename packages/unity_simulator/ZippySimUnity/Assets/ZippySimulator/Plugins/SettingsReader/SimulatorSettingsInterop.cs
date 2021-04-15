using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

namespace Zippy {

    [StructLayout(LayoutKind.Sequential)]
    public struct VehicleCalibration {
        public Vector2 center;
        public Vector2 frontRight;
        public Vector2 frontLeft;
        public Vector2 middleRight;
        public Vector2 middleLeft;
        public Vector2 rearRight;
        public Vector2 rearLeft;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct VehicleSettings {
        public readonly SimulatedVehicleModel model;
        public readonly VehicleCalibration vehicleCalibration;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct ApplicationSettings {
        public readonly int targetFrameRate;
        public readonly float targetFixedDeltaTime;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct CameraSettings {
        public readonly CameraId cameraId;
        int m_greyscale;
        public readonly float nearClippingPlaneDistanceMeters;
        public readonly float farClippingPlaneDistanceMeters;
        public readonly float distortedMeshRowFineness;
        public readonly float distortedMeshColFineness;
        public readonly float calibratedPlaneScaleFactor;
        public readonly float maxDepthMapDistanceMeters;
        public readonly float imageScale;
        public readonly float imageZoom;
        int m_enabled;
        int m_depthEnabled;
        int m_xyzEnabled;

        public bool greyscale {
            get {
                return m_greyscale > 0;
            }
        }
        public bool enabled {
            get {
                return m_enabled > 0;
            }
        }
        public bool depthEnabled {
            get {
                return m_depthEnabled > 0;
            }
        }
        public bool xyzEnabled {
            get {
                return m_xyzEnabled > 0;
            }
        }
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct CamerasSettings {
        public readonly int frameDownloadBufferSize;
        public readonly float cameraStartDepth;
        public readonly int maxTextureSize;
        [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.Struct, SizeConst = 8)]
        public readonly CameraSettings[] camera;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct NetworkingSettings {
        public readonly int zmqSendReceiveTimeoutMs;
        public readonly int zmqLingerTimeMs;
        public readonly int zmqHighWaterMark;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct GroundTruthSettings {
        int m_publishVehiclePosition;
        int m_publishFiducialPoses;

        public bool publishVehiclePosition {
            get {
                return m_publishVehiclePosition > 0;
            }
        }
        public bool publishFiducialPoses {
            get {
                return m_publishFiducialPoses > 0;
            }
        }
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct SceneSettings {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 2048)]  //allow for very long URLs and file paths. Not ideal, but limited by interop and strings in structs
        public readonly string sceneUrl;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SimulatorSettings {
        public readonly VehicleSettings vehicle;
        public readonly ApplicationSettings application;
        public readonly CamerasSettings cameras;
        public readonly NetworkingSettings networking;
        public readonly GroundTruthSettings groundTruth;
        public readonly SceneSettings scene;
    }
}
