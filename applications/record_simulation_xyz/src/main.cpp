#include "gflags/gflags.h"
#include "glog/logging.h"

#include "packages/core/include/producer_consumer_queue.h"
#include "packages/estimation/proto/state.pb.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/hal/proto/vcu_telemetry_envelope.pb.h"
#include "packages/net/include/zmq_select.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/serialization/include/protobuf_io.h"
#include "packages/unity_plugins/proto/unity_telemetry_envelope.pb.h"

#include <atomic>
#include <condition_variable>
#include <ctime>
#include <cxxabi.h>
#include <fstream>
#include <mutex>
#include <signal.h>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <typeinfo>
#include <unistd.h>

namespace {
DEFINE_string(cameraName, "FrontLeftStereo", "Name of the camera to capture");
DEFINE_uint32(cameraPort, 5503, "Port on which XYZ data for the camera is broadcast");
DEFINE_string(cameraTopic, "camera", "ZMQ topic on which camera XYZ data is being broadcast");
DEFINE_uint32(groundtruthTelemetryPort, 7501, "Port on which Unity simulator telemetry is published");
DEFINE_string(groundtruthTelemetryTopic, "telemetry", "ZMQ topic on which Unity simulator telemetry is published");
DEFINE_string(telemetryTopic, "telemetry", "VCU telemetry topic");
DEFINE_uint32(telemetryPort, 7001, "VCU telemetry port");
DEFINE_string(odometryTopic, "odometry", "Estimated wheel odometry topic");
DEFINE_uint32(odometryPort, 7100, "Estimated wheel odometry port");

void logCommandLineParameters() {
    LOG(INFO) << "Command line parameters:\n"
              << "  Camera name:                    [" << FLAGS_cameraName << "]\n"
              << "  Camera topic port:              [" << FLAGS_cameraPort << "]\n"
              << "  Camera topic:                   [" << FLAGS_cameraTopic << "]\n"
              << "  Ground truth pose port:         [" << FLAGS_groundtruthTelemetryPort << "]\n"
              << "  Ground truth pose topic:        [" << FLAGS_groundtruthTelemetryTopic << "]\n"
              << "  Servo telemetry port:           [" << FLAGS_telemetryPort << "]\n"
              << "  Servo telemetry topic:          [" << FLAGS_telemetryTopic << "]\n"
              << "  Estimated wheel odometry port:  [" << FLAGS_odometryPort << "]\n"
              << "  Estimated wheel odometry topic: [" << FLAGS_odometryTopic << "]";
}

std::atomic_bool quit(false);

void shouldQuit(int) { quit = true; }

std::string getDateTimeStamp() {
    auto now = ::time(nullptr);
    tm decomposedNow;
    ::gmtime_r(&now, &decomposedNow);
    constexpr size_t bufferSize = 1024;
    char buffer[bufferSize];
    ::strftime(buffer, bufferSize, "y%Y-m%m-d%d-H%H-M%M", &decomposedNow);
    return std::string(buffer);
}

std::string getGroundTruthPoseFileName() {
    std::stringstream fileName;
    fileName << getDateTimeStamp() << "-vehicle.pose";
    return fileName.str();
}

std::string getCameraFileName() {
    std::stringstream fileName;
    fileName << getDateTimeStamp() << "-" << FLAGS_cameraName << ".xyz";
    return fileName.str();
}

std::string getVCUTelemetryFileName() {
    std::stringstream fileName;
    fileName << getDateTimeStamp() << ".vcu.telemetry";
    return fileName.str();
}

std::string getWheelOdometryFileName() {
    std::stringstream fileName;
    fileName << getDateTimeStamp() << ".wheel.odometry";
    return fileName.str();
}

std::string demangle(const char* const name) {
    std::string result(name);

    char* buffer = nullptr;
    size_t bufferSize = 0;
    int status;

    auto tmp = abi::__cxa_demangle(name, buffer, &bufferSize, &status);

    if (0 == status) {
        result = tmp;
    }

    ::free(tmp);
    return result;
}

template <typename ITEM_TYPE> void writerThread(core::BoundedProducerConsumerBuffer<ITEM_TYPE>& queue, const std::string& fileName) {
    const auto demangledName = demangle(typeid(ITEM_TYPE).name());

    LOG(INFO) << "[TYPE: " << demangledName << " :: TID: " << std::this_thread::get_id() << "] Starting writer thread";

    LOG(INFO) << "[TYPE: " << demangledName << " :: TID: " << std::this_thread::get_id() << "] Writing vehicle ground truth poses to ["
              << fileName << "]";
    std::ofstream out(fileName.c_str(), std::ios::out | std::ios::binary);
    serialization::ProtobufWriter writer(&out);

    LOG(INFO) << "[TYPE: " << demangledName << " :: TID: " << std::this_thread::get_id() << "] Entering write loop";
    while (!quit) {
        ITEM_TYPE item;
        if (queue.dequeue(item)) {
            writer.writeNext(item);
        } else {
            LOG(INFO) << "[TYPE: " << demangledName << " :: TID: " << std::this_thread::get_id()
                      << "] Dequeue returned false, so someone wants us to shut down.";
            break;
        }
    }

    LOG(INFO) << "[TYPE: " << demangledName << " :: TID: " << std::this_thread::get_id() << "] Writer thread done";
}

std::shared_ptr<zmq::socket_t> createZmqSocket(const std::string& topic, const uint32_t port, zmq::context_t& context) {
    std::stringstream addressBuffer;
    addressBuffer << "tcp://localhost:" << port;

    LOG(INFO) << "Creating socket at address [" << addressBuffer.str() << "]";
    constexpr int receiveBufferSize = 3;
    auto result = std::make_shared<zmq::socket_t>(context, ZMQ_SUB);
    result->setsockopt(ZMQ_RCVHWM, receiveBufferSize);
    result->connect(addressBuffer.str());
    result->setsockopt(ZMQ_SUBSCRIBE, topic.data(), topic.size());
    LOG(INFO) << "Subscribing socket at [" << addressBuffer.str() << "] to topic [" << topic << "]";
    return result;
}
}

int main(int argc, char** argv) {
    // Install our signal handlers
    signal(SIGINT, &shouldQuit);
    signal(SIGQUIT, &shouldQuit);

    FLAGS_logtostderr = true;

    gflags::SetUsageMessage("Record x, y, z data and vehicle telemetry from the simulator");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    logCommandLineParameters();

    LOG(INFO) << "Spawning ground truth pose writing thread";
    constexpr size_t groundTruthPoseQueueLength = 500;
    core::BoundedProducerConsumerBuffer<unity_plugins::UnityTelemetryEnvelope> groundTruthPoseQueue(groundTruthPoseQueueLength);
    std::thread groundTruthPoseThread([&groundTruthPoseQueue]() { writerThread(groundTruthPoseQueue, getGroundTruthPoseFileName()); });

    LOG(INFO) << "Spawning xyz writing thread";
    constexpr size_t xyzQueueDepth = 5;
    core::BoundedProducerConsumerBuffer<hal::CameraSample> xyzQueue(xyzQueueDepth);
    std::thread xyzThread([&]() { writerThread(xyzQueue, getCameraFileName()); });

    LOG(INFO) << "Spawning VCU telemetry writing thread";
    constexpr size_t vcuTelemetryQueueLength = 500;
    core::BoundedProducerConsumerBuffer<hal::VCUTelemetryEnvelope> vcuTelemetryQueue(vcuTelemetryQueueLength);
    std::thread vcuTelemetryThread([&vcuTelemetryQueue]() { writerThread(vcuTelemetryQueue, getVCUTelemetryFileName()); });

    LOG(INFO) << "Spawning wheel odometry writing thread";
    constexpr size_t wheelOdometryQueueLength = 500;
    core::BoundedProducerConsumerBuffer<estimation::StateProto> wheelOdometryQueue(wheelOdometryQueueLength);
    std::thread wheelOdometryThread([&wheelOdometryQueue]() { writerThread(wheelOdometryQueue, getWheelOdometryFileName()); });

    const auto numCores = std::thread::hardware_concurrency();
    LOG(INFO) << "Creating ZMQ context";
    zmq::context_t context(numCores / 4 + 1);

    net::ZMQSelectLoop select;

    LOG(INFO) << "Setting up receive of unity ground truth telemetry";
    auto groundTruthSocket = createZmqSocket(FLAGS_groundtruthTelemetryTopic, FLAGS_groundtruthTelemetryPort, context);
    select.OnProtobuf<unity_plugins::UnityTelemetryEnvelope>(*groundTruthSocket, FLAGS_groundtruthTelemetryTopic,
        [&groundTruthPoseQueue](const unity_plugins::UnityTelemetryEnvelope& e) { groundTruthPoseQueue.enqueue(e); });

    LOG(INFO) << "Setting up receive of xyz data from Unity";
    auto xyzSocket = createZmqSocket(FLAGS_cameraTopic, FLAGS_cameraPort, context);
    select.OnProtobuf<hal::CameraSample>(*xyzSocket, FLAGS_cameraTopic, [&xyzQueue](const hal::CameraSample& s) {
        LOG(INFO) << "IMAGE: TYPE=[" << s.image().type() << "], FORMAT=[" << s.image().format() << "]";
        xyzQueue.enqueue(s);
    });

    LOG(INFO) << "Setting up receive of VCU telemetry";
    auto vcuTelemetrySocket = createZmqSocket(FLAGS_telemetryTopic, FLAGS_telemetryPort, context);
    select.OnProtobuf<hal::VCUTelemetryEnvelope>(*vcuTelemetrySocket, FLAGS_telemetryTopic,
        [&vcuTelemetryQueue](const hal::VCUTelemetryEnvelope& e) { vcuTelemetryQueue.enqueue(e); });

    LOG(INFO) << "Setting up receive of wheel odometry";
    auto wheelOdometrySocket = createZmqSocket(FLAGS_odometryTopic, FLAGS_odometryPort, context);
    select.OnProtobuf<estimation::StateProto>(*wheelOdometrySocket, FLAGS_odometryTopic,
        [&wheelOdometryQueue](const estimation::StateProto& state) { wheelOdometryQueue.enqueue(state); });

    LOG(INFO) << "Entering main IO loop";
    while (!quit) {
        try {
            select.Poll();
        } catch (zmq::error_t& e) {
            LOG(ERROR) << "Select loop threw an exception: [" << e.what() << "]";
        }
    }

    LOG(INFO) << "Issuing shutdown to queues";
    groundTruthPoseQueue.shutdown();
    xyzQueue.shutdown();
    vcuTelemetryQueue.shutdown();
    wheelOdometryQueue.shutdown();

    LOG(INFO) << "Joining ground truth pose thread";
    groundTruthPoseThread.join();

    LOG(INFO) << "Joining xyz thread";
    xyzThread.join();

    LOG(INFO) << "Joining VCU telemetry thread";
    vcuTelemetryThread.join();

    LOG(INFO) << "Joining wheel odometry thread";
    wheelOdometryThread.join();

    return 0;
}