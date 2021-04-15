#include <math.h>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <unistd.h>

#include "gflags/gflags.h"

#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/net/include/zmq_topic_pub.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/perception/grid.h"
#include "packages/perception/logging.h"
#include "packages/perception/perception.h"
#include "packages/planning/utils.h"

DEFINE_string(mode, "", "Mode to execute (synthetic, file, live)");
DEFINE_string(dataFile, "", "Stereo file to ingest, if applicable");
DEFINE_string(subscriberAddress, "tcp://localhost:5557", "Point provider address to subscribe to");
DEFINE_string(subscribeTopic, "stereo", "Points topic");
DEFINE_string(publisherAddress, "tcp://*:6550", "Output topic for voxel data");
DEFINE_string(publisherTopic, "voxels", "Output topic for voxel data");

using perception::VoxelGrid;
using perception::VoxelGridOptions;
using perception::VoxelGridProto;
using perception::Point3f;
using perception::PointCloudXYZ;

class VoxelOutput {
public:
    VoxelOutput(const std::string& address, const std::string& topic)
        : m_topic(topic) {
        m_comms.reset(new net::ZMQProtobufPublisher<VoxelGridProto>(m_context, address, 1, -1));
    }

    void send(const VoxelGrid& grid) {
        VoxelGridProto proto;
        auto start = std::chrono::high_resolution_clock::now();
        perception::serialize(grid, proto);
        auto serialize = std::chrono::high_resolution_clock::now();
        CHECK(m_comms->send(proto, m_topic));
        auto send = std::chrono::high_resolution_clock::now();
        LOG(INFO) << std::chrono::duration_cast<std::chrono::milliseconds>(serialize - start).count() << " (serialize) ms";
        LOG(INFO) << std::chrono::duration_cast<std::chrono::milliseconds>(send - serialize).count() << " (send) ms";
    }

private:
    std::string m_topic;
    zmq::context_t m_context = zmq::context_t(1);
    std::unique_ptr<net::ZMQProtobufPublisher<perception::VoxelGridProto> > m_comms;
};

void synthetic_voxel_generator() {
    CHECK(!FLAGS_publisherAddress.empty()) << "Invalid/missing address: " << FLAGS_publisherAddress;
    CHECK(!FLAGS_publisherTopic.empty()) << "Invalid/missing topic: " << FLAGS_publisherTopic;
    VoxelOutput output(FLAGS_publisherAddress, FLAGS_publisherTopic);

    constexpr float half_grid = 5;
    VoxelGridOptions options;
    options.mutable_grid_options()->set_dim_x(2 * half_grid);
    options.mutable_grid_options()->set_dim_y(2 * half_grid);
    options.mutable_grid_options()->set_dim_z(2 * half_grid);
    options.mutable_grid_options()->set_res_x(.5);
    options.mutable_grid_options()->set_res_y(.5);
    options.mutable_grid_options()->set_res_z(.5);

    VoxelGrid grid(options);
    while (true) {
        for (float i = -half_grid; i < half_grid; i += 0.1) {
            for (float j = -half_grid; j < half_grid; j += 0.1) {
                for (float k = -half_grid; k < half_grid; k += 0.1) {
                    PointCloudXYZ cloud;
                    cloud.xyz = Eigen::MatrixXf(3, 1);
                    cloud.xyz.block<3, 1>(0, 0) = Point3f({ i, j, k });
                    CHECK(grid.add(cloud));
                }
            }
        }
        grid.computeSurflets();
        output.send(grid);
    }
}

void file_voxel_generator() {
    CHECK(!FLAGS_publisherAddress.empty()) << "Invalid/missing address: " << FLAGS_publisherAddress;
    CHECK(!FLAGS_publisherTopic.empty()) << "Invalid/missing topic: " << FLAGS_publisherTopic;
    VoxelOutput output(FLAGS_publisherAddress, FLAGS_publisherTopic);
    CHECK(!FLAGS_dataFile.empty()) << "Invalid log input";
    perception::LogReader<hal::CameraSample> reader(FLAGS_dataFile);

    hal::CameraSample sample;
    std::deque<hal::CameraSample> samples;
    while (reader.read(sample)) {
        samples.emplace_back(sample);
    }
    CHECK(samples.size() > 0) << "Failed to read any stereo data";

    using perception::VoxelTerrain;
    using perception::VoxelTerrainOptions;

    VoxelTerrainOptions voxel_options;
    voxel_options.mutable_voxel_grid_options()->mutable_grid_options()->set_dim_x(10.0);
    voxel_options.mutable_voxel_grid_options()->mutable_grid_options()->set_dim_y(10.0);
    voxel_options.mutable_voxel_grid_options()->mutable_grid_options()->set_dim_z(10.0);
    voxel_options.mutable_voxel_grid_options()->mutable_grid_options()->set_res_x(0.2);
    voxel_options.mutable_voxel_grid_options()->mutable_grid_options()->set_res_y(0.2);
    voxel_options.mutable_voxel_grid_options()->mutable_grid_options()->set_res_z(0.2);
    std::unique_ptr<VoxelTerrain> terrain(new VoxelTerrain(voxel_options));
    auto& grid = terrain->grid();
    PointCloudXYZ cloud;

    while (true) {
        auto sample = samples.front();
        CHECK(imageToPointCloud(sample.image(), cloud));
        grid.clear();
        terrain->addPoints(cloud);
        output.send(grid);
        // Rotate deque (this is faster than std::rotate)
        samples.emplace_back(samples.front());
        samples.pop_front();
    }
}

void live_voxel_generator() {
    CHECK(!FLAGS_publisherAddress.empty()) << "Invalid/missing address: " << FLAGS_publisherAddress;
    CHECK(!FLAGS_publisherTopic.empty()) << "Invalid/missing topic: " << FLAGS_publisherTopic;
    VoxelOutput output(FLAGS_publisherAddress, FLAGS_publisherTopic);
    CHECK(!FLAGS_subscriberAddress.empty());
    using perception::VoxelTerrain;
    using perception::VoxelTerrainOptions;

    VoxelTerrainOptions voxel_options;
    voxel_options.mutable_voxel_grid_options()->mutable_grid_options()->set_dim_x(20.0);
    voxel_options.mutable_voxel_grid_options()->mutable_grid_options()->set_dim_y(20.0);
    voxel_options.mutable_voxel_grid_options()->mutable_grid_options()->set_dim_z(20.0);
    voxel_options.mutable_voxel_grid_options()->mutable_grid_options()->set_res_x(0.2);
    voxel_options.mutable_voxel_grid_options()->mutable_grid_options()->set_res_y(0.2);
    voxel_options.mutable_voxel_grid_options()->mutable_grid_options()->set_res_z(0.2);
    std::unique_ptr<VoxelTerrain> terrain(new VoxelTerrain(voxel_options));
    auto& grid = terrain->grid();
    perception::PointCloudXYZ cloud;

    zmq::context_t context(1);
    LOG(INFO) << FLAGS_subscriberAddress << ", " << FLAGS_subscribeTopic;
    net::ZMQProtobufSubscriber<hal::CameraSample> subscriber(context, FLAGS_subscriberAddress, FLAGS_subscribeTopic, 1);

    bool strict = true;

    Eigen::AngleAxisd rotation(M_PI / 2.0, Eigen::Vector3d{ 1, 0, 0 });
    Sophus::SE3d cameraToZippy(Eigen::Quaterniond(rotation), Eigen::Vector3d{ 0, 0, 0 });

    hal::CameraSample sample;
    while (true) {
        if (subscriber.poll()) {
            hal::CameraSample sample;
            CHECK(subscriber.recv(sample));
            CHECK(sample.image().format() == hal::Format::PB_POINTCLOUD);
            // Convert the image xyz format into a dense cloud
            imageToPointCloud(sample.image(), cloud);
            // Transform from native camera frame to zippy camera frame
            transform(cloud, cameraToZippy);
            if (strict) {
                checkCloud(cloud, sample.image().rows() * sample.image().cols());
            }
            grid.clear();
            terrain->addPoints(cloud);
            output.send(grid);
        }
    }
}

int main(int argc, char** argv) {
    gflags::SetUsageMessage("Voxelizer");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, &argv, false);
    std::thread worker;
    if (FLAGS_mode == "synthetic") {
        worker = std::thread(synthetic_voxel_generator);
    } else if (FLAGS_mode == "file") {
        CHECK(!FLAGS_dataFile.empty()) << "Require data file";
        worker = std::thread(file_voxel_generator);
    } else if (FLAGS_mode == "live") {
        CHECK(!FLAGS_subscriberAddress.empty()) << "Require subscriber address";
        CHECK(!FLAGS_subscribeTopic.empty()) << "Require topic";
        worker = std::thread(live_voxel_generator);
    } else {
        LOG(FATAL) << "Unknown/invalid mode: (" << FLAGS_mode << ")";
    }
    worker.join();
    return EXIT_SUCCESS;
}
