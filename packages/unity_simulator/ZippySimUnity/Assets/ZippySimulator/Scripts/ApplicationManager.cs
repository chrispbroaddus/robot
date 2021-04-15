using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Zippy {
    /// <summary>
    /// Application manager. This coordinates messaging, and configures the app
    /// </summary>
    public class ApplicationManager : MonoBehaviour {
        // Use this for initialization
        void Start () {
            GlogWrapper.Init ();
            ConfigureApp ();
            ConfigureMessaging ();
        }

        void ConfigureApp() {
            //This should be set in the quality settings already, but set here to to ensure it is applied
            //Not syncing to vblank makes the transfer bandwidth unrestrcited!
            QualitySettings.vSyncCount = 0;
            //set the target framerate
            Application.targetFrameRate = SimulatorSettingsManager.Settings.application.targetFrameRate;
            Time.fixedDeltaTime = SimulatorSettingsManager.Settings.application.targetFixedDeltaTime;
        }

        void ConfigureMessaging() {
            ZippyEventManager.StartListening (ZippyEventType.CamerasReady, HandleZippyEvents);
            ZippyEventManager.StartListening (ZippyEventType.RobotReady, HandleZippyEvents);
        }

        void Initialize() {
            CameraRigManager.Instance.EnableCapture = true;
            ImageStreamer.Instance.EnableStreamToNetwork = true;
            CommandManager.Instance.OnEnableCamera += CameraRigManager.Instance.EnableCamera;
            CommandManager.Instance.OnEnableDepthOutput += CameraRigManager.Instance.EnableDepthImage;
            CommandManager.Instance.OnEnableXYZOutput += CameraRigManager.Instance.EnableXYZImage;
            CommandManager.Instance.OnResetWorld += HandleResetWorld;
            CommandManager.Instance.OnResetStreams += HandleResetStreams;
        }

        void HandleResetStreams() {
            CameraRigManager.Instance.EnableCameras (false);
            CameraRigManager.Instance.EnableDepthImages (false);
            CameraRigManager.Instance.EnableXYZImages (false);
        }

        void HandleResetWorld() {
            Debug.Log("Resetting world");
            //TODO reset the world, when we have things that need resetting
        }

        void HandleZippyEvents(ZippyEventType zippyEvent, ZippyEventPayloadBase payload) {
            switch (zippyEvent) {
            case ZippyEventType.CamerasReady:
                Initialize ();
                break;

            case ZippyEventType.RobotReady:
                var zippyBot = FindObjectOfType<ZippyBotWheelsControllerUI> ();

                if (zippyBot != null) {
                    CommandManager.Instance.OnResetRobot += zippyBot.Reset;
                }

                break;

            default:
                break;
            }
        }



    }
}
