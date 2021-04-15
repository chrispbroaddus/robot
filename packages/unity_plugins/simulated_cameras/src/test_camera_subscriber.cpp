//
// Created by byungsookim on 5/14/17.
//

#include "glog/logging.h"
#include "packages/hal/proto/camera_id.pb.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/unity_plugins/image_saver/include/image_saver.h"
#include "packages/unity_plugins/simulated_cameras/include/simulator_publisher_topics.h"

#include <cstdlib>
#include <zmq.hpp>

using namespace unity_plugins;

///
/// @brief Run the camera subscriber for testing purposes
///
int main(int argc, char** argv) {

    google::InitGoogleLogging(argv[0]);

    if (argc < 3) {
        LOG(ERROR) << "USAGE: " << argv[0] << " IP_ADDRESS PORT_NUMBER";
        LOG(ERROR) << "\t Append 'save' to save images to disk";
        return EXIT_FAILURE;
    }

    std::string ipAddress(argv[1]);
    std::string portStr(argv[2]);

    bool save = false;
    if (argc >= 4 && strcmp(argv[3], "save") == 0) {
        save = true;
    }

    //  Prepare our context and subscriber
    zmq::context_t context(1);
    std::string addr = "tcp://" + ipAddress + ":" + portStr;
    net::ZMQProtobufSubscriber<hal::CameraSample> subscriber(context, addr, CAMERA_OUTPUT_TOPIC, 1);

    LOG(INFO) << "subscriber connected.";

    int frameNumber = 0;

    while (1) {
        hal::CameraSample cameraSample;
        if (subscriber.recv(cameraSample)) {
            LOG(INFO) << "message received from " << cameraSample.device().name() << ", size : " << cameraSample.image().cols() << "x"
                      << cameraSample.image().rows();

            if (save) {
                CameraImage cameraImage;

                if (!hal::CameraId_Parse(cameraSample.device().name(), &cameraImage.cameraId)) {
                    LOG(ERROR) << "Failed to parse camera name " << cameraSample.device().name();
                }

                if (cameraSample.image().format() == hal::Format::PB_RGB && cameraSample.image().type() == hal::Type::PB_UNSIGNED_BYTE) {
                    cameraImage.imageType = ImageType::Color;
                    cameraImage.bytesPerPixel = 3;
                } else if (cameraSample.image().format() == hal::Format::PB_LUMINANCE
                    && cameraSample.image().type() == hal::Type::PB_UNSIGNED_BYTE) {
                    cameraImage.imageType = ImageType::Greyscale;
                    cameraImage.bytesPerPixel = 1;
                } else if (cameraSample.image().format() == hal::Format::PB_LUMINANCE
                    && cameraSample.image().type() == hal::Type::PB_FLOAT) {
                    cameraImage.imageType = ImageType::Depth;
                    cameraImage.bytesPerPixel = 4;
                } else if (cameraSample.image().format() == hal::Format::PB_POINTCLOUD
                    && cameraSample.image().type() == hal::Type::PB_FLOAT) {
                    cameraImage.imageType = ImageType::Pointcloud;
                    cameraImage.bytesPerPixel = 12;
                }

                int imageSize = sizeof(unsigned char) * cameraSample.image().rows() * cameraSample.image().stride();
                cameraImage.data = (unsigned char*)malloc(imageSize);
                memcpy(cameraImage.data, cameraSample.image().data().data(), imageSize);
                cameraImage.width = cameraSample.image().cols();
                cameraImage.height = cameraSample.image().rows();

                cameraImage.frameNumber = frameNumber++;
                cameraImage.timestamp = 0;

                SaveImage(cameraImage);

                free(cameraImage.data);
            }
        }
    }
    return 0;
}
