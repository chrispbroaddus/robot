#include "packages/unity_plugins/simulator_settings_reader/include/simulator_settings_reader.h"
#include "glog/logging.h"
#include "packages/unity_plugins/simulator_settings_reader/include/default_simulator_settings.h"
#include "gtest/gtest.h"
#include <fstream>
#include <google/protobuf/util/json_util.h>
#include <string>

using namespace unity_plugins;

TEST(SimulatorSettingsReader, ctor) {

    const std::string testJsonFile("/tmp/simulator_settings.json");

    // write out the default settings to a temp file
    simulator_settings::SimulatorSettings defaultSettings = defaultSimulatorSettings();
    std::string msg;
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    google::protobuf::util::MessageToJsonString(defaultSettings, &msg, options);

    std::ofstream out(testJsonFile);
    out << msg;
    out.close();

    // read in settings
    SimulatorSettingsReader reader(testJsonFile);

    // compare

    simulator_settings::SimulatorSettings settings = reader.settings();
    SimulatorSettings interopSettings = reader.interopSettings();

    // vehicle
    EXPECT_EQ(defaultSettings.vehicle().model_id(), settings.vehicle().model_id());
    EXPECT_EQ(defaultSettings.vehicle().model_id(), interopSettings.vehicle.modelId);

    // application
    EXPECT_EQ(defaultSettings.application().targetframerate(), settings.application().targetframerate());
    EXPECT_EQ(defaultSettings.application().targetframerate(), interopSettings.application.targetFrameRate);

    EXPECT_NEAR(defaultSettings.application().targetfixeddeltatime(), settings.application().targetfixeddeltatime(), 1e-4);
    EXPECT_NEAR(defaultSettings.application().targetfixeddeltatime(), interopSettings.application.targetFixedDeltaTime, 1e-4);

    // cameras sectiojn
    EXPECT_EQ(defaultSettings.cameras().framedownloadbuffersize(), settings.cameras().framedownloadbuffersize());
    EXPECT_EQ(defaultSettings.cameras().framedownloadbuffersize(), interopSettings.cameras.frameDownloadBufferSize);

    EXPECT_NEAR(defaultSettings.cameras().camerastartdepth(), settings.cameras().camerastartdepth(), 1e-4);
    EXPECT_NEAR(defaultSettings.cameras().camerastartdepth(), interopSettings.cameras.cameraStartDepth, 1e-4);

    EXPECT_EQ(defaultSettings.cameras().maxtexturesize(), settings.cameras().maxtexturesize());
    EXPECT_EQ(defaultSettings.cameras().maxtexturesize(), interopSettings.cameras.maxTextureSize);

    EXPECT_EQ(defaultSettings.cameras().camera().size(), settings.cameras().camera().size());
    EXPECT_EQ(defaultSettings.cameras().camera().size(), sizeof(interopSettings.cameras) / sizeof(CameraSettings));

    // camera section
    for (int ii = 0; ii < defaultSettings.cameras().camera().size(); ++ii) {
        auto& defaultCam = defaultSettings.cameras().camera(ii);
        auto& cam = settings.cameras().camera(ii);
        auto& camInterop = interopSettings.cameras.camera[ii];

        EXPECT_EQ(defaultCam.cameraid(), cam.cameraid());
        EXPECT_EQ(defaultCam.cameraid(), camInterop.cameraId);

        EXPECT_EQ(defaultCam.greyscale(), cam.greyscale());
        EXPECT_EQ(defaultCam.greyscale(), camInterop.greyscale);

        EXPECT_NEAR(defaultCam.nearclippingplanedistancemeters(), cam.nearclippingplanedistancemeters(), 1e-4);
        EXPECT_NEAR(defaultCam.nearclippingplanedistancemeters(), camInterop.nearClippingPlaneDistanceMeters, 1e-4);

        EXPECT_NEAR(defaultCam.farclippingplanedistancemeters(), cam.farclippingplanedistancemeters(), 1e-4);
        EXPECT_NEAR(defaultCam.farclippingplanedistancemeters(), camInterop.farClippingPlaneDistanceMeters, 1e-4);

        EXPECT_NEAR(defaultCam.distortedmeshrowfineness(), cam.distortedmeshrowfineness(), 1e-4);
        EXPECT_NEAR(defaultCam.distortedmeshrowfineness(), camInterop.distortedMeshRowFineness, 1e-4);

        EXPECT_NEAR(defaultCam.distortedmeshcolfineness(), cam.distortedmeshcolfineness(), 1e-4);
        EXPECT_NEAR(defaultCam.distortedmeshcolfineness(), camInterop.distortedMeshColFineness, 1e-4);

        EXPECT_NEAR(defaultCam.calibratedplanescalefactor(), cam.calibratedplanescalefactor(), 1e-4);
        EXPECT_NEAR(defaultCam.calibratedplanescalefactor(), camInterop.calibratedPlaneScaleFactor, 1e-4);

        EXPECT_NEAR(defaultCam.maxdepthmapdistancemeters(), cam.maxdepthmapdistancemeters(), 1e-4);
        EXPECT_NEAR(defaultCam.maxdepthmapdistancemeters(), camInterop.maxDepthMapDistanceMeters, 1e-4);

        EXPECT_NEAR(defaultCam.imagescale(), cam.imagescale(), 1e-4);
        EXPECT_NEAR(defaultCam.imagescale(), camInterop.imageScale, 1e-4);

        EXPECT_NEAR(defaultCam.imagezoom(), cam.imagezoom(), 1e-4);
        EXPECT_NEAR(defaultCam.imagezoom(), camInterop.imageZoom, 1e-4);

        EXPECT_EQ(defaultCam.enabled(), cam.enabled());
        EXPECT_EQ(defaultCam.enabled(), camInterop.enabled);

        EXPECT_EQ(defaultCam.depthenabled(), cam.depthenabled());
        EXPECT_EQ(defaultCam.depthenabled(), camInterop.depthEnabled);

        EXPECT_EQ(defaultCam.xyzenabled(), cam.xyzenabled());
        EXPECT_EQ(defaultCam.xyzenabled(), camInterop.pointcloudEnabled);
    }

    // networking section
    EXPECT_EQ(defaultSettings.networking().zmqsendreceivetimeoutms(), settings.networking().zmqsendreceivetimeoutms());
    EXPECT_EQ(defaultSettings.networking().zmqsendreceivetimeoutms(), interopSettings.networking.zmqSendReceiveTimeoutMs);

    EXPECT_EQ(defaultSettings.networking().zmqlingertimems(), settings.networking().zmqlingertimems());
    EXPECT_EQ(defaultSettings.networking().zmqlingertimems(), interopSettings.networking.zmqLingerTimeMs);

    EXPECT_EQ(defaultSettings.networking().zmqhighwatermark(), settings.networking().zmqhighwatermark());
    EXPECT_EQ(defaultSettings.networking().zmqhighwatermark(), interopSettings.networking.zmqHighWaterMark);

    // ground truth section
    EXPECT_EQ(defaultSettings.groundtruth().publishvehicleposition(), settings.groundtruth().publishvehicleposition());
    EXPECT_EQ(defaultSettings.groundtruth().publishvehicleposition(), interopSettings.groundTruth.publishVehiclePosition);

    EXPECT_EQ(defaultSettings.groundtruth().publishfiducialposes(), settings.groundtruth().publishfiducialposes());
    EXPECT_EQ(defaultSettings.groundtruth().publishfiducialposes(), interopSettings.groundTruth.publishFiducialPoses);

    // scene section
    EXPECT_EQ(defaultSettings.scene().sceneurl(), settings.scene().sceneurl());
    EXPECT_EQ(defaultSettings.scene().sceneurl(), interopSettings.scene.sceneUrl);
    EXPECT_EQ(0, defaultSettings.scene().sceneurl().size());
}
