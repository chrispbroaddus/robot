#include "gflags/gflags.h"
#include "glog/logging.h"

#include "packages/benchmarking/include/summary_statistics.h"
#include "packages/core/include/pose_interpolator.h"
#include "packages/core/include/producer_consumer_queue.h"
#include "packages/dense_mapping/include/insert_policies.h"
#include "packages/dense_mapping/include/octree.h"
#include "packages/dense_mapping/include/octree_payloads.h"
#include "packages/dense_mapping/include/octree_serialization.h"
#include "packages/dense_mapping/include/octree_serialization_specializations.h"
#include "packages/dense_mapping/proto/volumetric_time_series_dataset.pb.h"
#include "packages/estimation/proto/state.pb.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/math/geometry/se3.h"
#include "packages/serialization/include/protobuf_io.h"
#include "packages/unity_plugins/proto/unity_telemetry_envelope.pb.h"

#include <fstream>
#include <future>
#include <map>
#include <sstream>
#include <stdexcept>

namespace {
using scalar_type = float;
using occupied_only_octree_type = dense_mapping::Octree<scalar_type, dense_mapping::payloads::OccupiedOnlyOctreeLeafNode,
    dense_mapping::insert_policies::TraverseBoxInsertPolicy>;
using clock_type = std::chrono::high_resolution_clock;

DEFINE_string(unityGroundTruthPoseFile, "", "File containing Unity ground truth telemetry envelope data");
DEFINE_string(telemetryPoseFile, "", "File containing odometry poses");
DEFINE_string(cameraDataFile, "", "File containing Unity ground truth point clouds");
DEFINE_string(outputFile, "model.octree", "Output model file");
DEFINE_uint32(octreeDepth, 8, "Maximum octree depth");
DEFINE_double(octreeHalfSpan, 16, "Octree half-span (half the length of a side of the cubic volume we will be modeling)");

void logCommandLineParameters() {
    LOG(INFO) << "Command line parameters:\n"
              << "  Unity ground truth vehicle pose file: [" << FLAGS_unityGroundTruthPoseFile << "]\n"
              << "  Telemetry pose file:                  [" << FLAGS_telemetryPoseFile << "]\n"
              << "  Camera data file:                     [" << FLAGS_cameraDataFile << "]\n"
              << "  Output file:                          [" << FLAGS_outputFile << "]\n"
              << "  Octree depth:                         [" << FLAGS_octreeDepth << "]\n"
              << "  Octree half span:                     [" << FLAGS_octreeHalfSpan << "]";
}

std::map<uint64_t, Sophus::SE3<scalar_type> > loadPosesFromGroundTruth(const std::string& fileName) {
    LOG(INFO) << "Loading poses from [" << fileName << "]";
    std::ifstream in(fileName.c_str(), std::ios::in | std::ios::binary);
    serialization::ProtobufReader reader(&in);

    unity_plugins::UnityTelemetryEnvelope envelope;

    std::map<uint64_t, Sophus::SE3<scalar_type> > result;

    while (reader.readNext(envelope)) {
        const uint64_t timestamp = envelope.vehiclepose().measurementsystemtimestamp().nanos();
        geometry::getSophusSE3(result[timestamp], envelope.vehiclepose().transformations());
    }

    LOG(INFO) << "Loaded [" << result.size() << "] ground truth poses from [" << fileName << "]";

    return result;
}

std::map<uint64_t, Sophus::SE3<scalar_type> > loadPosesFromOdometry(const std::string& fileName) {
    LOG(INFO) << "Loading poses from [" << fileName << "]";
    std::ifstream in(fileName.c_str(), std::ios::in | std::ios::binary);
    serialization::ProtobufReader reader(&in);

    estimation::StateProto envelope;
    std::map<uint64_t, Sophus::SE3<scalar_type> > result;

    while (reader.readNext(envelope)) {
        const uint64_t timestamp = envelope.hardware_timestamp().nanos();
        geometry::getSophusSE3(result[timestamp], envelope.transform());
    }

    LOG(INFO) << "Loaded [" << result.size() << "] ground truth poses from [" << fileName << "]";

    return result;
}

void sanityCheckImage(const hal::CameraSample& sample) {
    if (!sample.has_hardwaretimestamp()) {
        throw std::runtime_error("Camera sample does not have a hardware timestamp set");
    }

    if (0 == sample.hardwaretimestamp().nanos()) {
        throw std::runtime_error("Camera sample has invalid hardware timestamp");
    }

    if (!sample.has_systemtimestamp()) {
        throw std::runtime_error("Camera sample does not have system timestamp set");
    }

    if (0 == sample.systemtimestamp().nanos()) {
        throw std::runtime_error("Camera sample has invalid system timestamp");
    }

    if (!sample.has_image()) {
        throw std::runtime_error("Camera sample does not have image set");
    }

    if ((sample.image().type() != hal::PB_FLOAT)) {
        LOG(ERROR) << "Got image type [" << sample.image().type() << "], expected PB_FLOAT";
        throw std::runtime_error("Camera sample image has unexpected type");
    }

    if (sample.image().format() != hal::PB_POINTCLOUD) {
        LOG(ERROR) << "Got image type [" << sample.image().format() << "], expected PB_POINTCLOUD";
        throw std::runtime_error("Camera sample image does not have the expected format");
    }

    constexpr size_t channelsPerPixel = 3;
    constexpr size_t bytesPerPixel = sizeof(float) * channelsPerPixel;
    const size_t numPixels = sample.image().rows() * sample.image().cols();
    const size_t numBytes = bytesPerPixel * numPixels;
    if (sample.image().data().size() != numBytes) {
        LOG(ERROR) << "Expected a total of [" << numBytes << "], got [" << sample.image().data().size() << "] instead.";
        throw std::runtime_error("Unexpected size for image data");
    }
}

constexpr scalar_type getElapsedTimeInSeconds(int64_t startTimeNanos, int64_t currentTimeNanos) {
    constexpr int64_t nanosecondsPerSecond = 1000000000LL;
    return static_cast<scalar_type>(currentTimeNanos - startTimeNanos) / nanosecondsPerSecond;
}

struct VolumetricTimeSeriesState {
    SummaryStatistics<scalar_type> mergeStatistics;
    SummaryStatistics<scalar_type> insertStatistics;
    dense_mapping::VolumetricTimeSeries series;
};

std::shared_ptr<const core::PoseInterpolator<scalar_type> > createInterpolator(
    uint64_t initialTime, const std::map<uint64_t, Sophus::SE3<scalar_type> >& poseMap) {
    std::map<scalar_type, Sophus::SE3<scalar_type> > convertedTimes;

    for (const auto& sample : poseMap) {
        convertedTimes.insert(std::make_pair(getElapsedTimeInSeconds(initialTime, sample.first), sample.second));
    }

    return std::make_shared<core::PoseInterpolator<scalar_type> >(convertedTimes);
}

void transformPoints(std::vector<std::array<scalar_type, 3> >& transformedPoints, const hal::CameraSample& sample,
    const Sophus::SE3<scalar_type>& vehicleToModel) {
    // Try to manage this buffer in such a way that it sticks around and we're not churning memory
    const auto points = sample.image().cols() * sample.image().rows();
    transformedPoints.reserve(std::max<size_t>(transformedPoints.capacity(), points));
    transformedPoints.resize(points);

    constexpr size_t channelsPerPixel = 3;
    constexpr size_t bytesPerChannel = sizeof(float);
    constexpr size_t pixelStrideBytes = channelsPerPixel * bytesPerChannel;
    const size_t bytesPerRow = pixelStrideBytes * sample.image().cols();
    const char* data = sample.image().data().data();

    size_t i = 0;
    for (size_t r = 0; r < sample.image().rows(); ++r) {
        const char* const row = &data[r * bytesPerRow];

        for (size_t c = 0; c < sample.image().cols(); ++c) {
            std::array<float, 3> point{ { 0, 0, 0 } };
            std::memcpy(reinterpret_cast<char*>(point.data()), row + c * pixelStrideBytes, pixelStrideBytes);

            Sophus::SE3<scalar_type>::Point p;
            p[0] = point[0];
            p[1] = point[1];
            p[2] = point[2];

            const auto transformed = vehicleToModel * p;
            transformedPoints.at(i)[0] = transformed[0];
            transformedPoints.at(i)[1] = transformed[1];
            transformedPoints.at(i)[2] = transformed[2];
            ++i;
        }
    }
}

dense_mapping::VolumetricModel* extractModel(const occupied_only_octree_type& tree) {
    auto result = new dense_mapping::VolumetricModel;

    dense_mapping::serialize(tree, *result);

    return result;
}

void buildTimeSeries(core::BoundedProducerConsumerBuffer<std::shared_ptr<const hal::CameraSample> >& pointQueue,
    const std::map<uint64_t, Sophus::SE3<scalar_type> >& poseMap, std::promise<VolumetricTimeSeriesState> result) {
    LOG(INFO) << "WORKER THREAD[" << std::this_thread::get_id() << "] STARTED";

    occupied_only_octree_type integratedModel(FLAGS_octreeHalfSpan, FLAGS_octreeDepth);

    VolumetricTimeSeriesState output;
    std::shared_ptr<const hal::CameraSample> sample;

    std::shared_ptr<const core::PoseInterpolator<scalar_type> > interpolator;

    uint64_t initialTime = 0xFFFFFFFFFFFFFFFFULL;

    std::vector<std::array<scalar_type, 3> > transformedPoints;

    uint64_t frameCount = 0;

    while (pointQueue.dequeue(sample)) {
        LOG(INFO) << "WORKER THREAD[" << std::this_thread::get_id() << "] PROCESSING FRAME [# " << frameCount
                  << " / HWTS: " << sample->hardwaretimestamp().nanos() << "] @ [" << sample.get() << "]";

        try {
            occupied_only_octree_type partialModel(FLAGS_octreeHalfSpan, FLAGS_octreeDepth);

            if (!interpolator) {
                initialTime = std::min(sample->hardwaretimestamp().nanos(), poseMap.begin()->first);
                interpolator = createInterpolator(initialTime, poseMap);
            }

            const auto currentDataTime = getElapsedTimeInSeconds(initialTime, sample->hardwaretimestamp().nanos());
            const auto currentPose = interpolator->interpolateAtTime(currentDataTime);

            transformPoints(transformedPoints, *sample, currentPose);

            // Track how long it takes for us to insert points into an empty model
            const std::array<scalar_type, 3> sensorOrigin{ { currentPose.translation()[0], currentPose.translation()[1],
                currentPose.translation()[2] } };
            {
                auto start = clock_type::now();
                for (const auto& pt : transformedPoints) {
                    partialModel.insert(sensorOrigin, pt);
                }
                output.insertStatistics.update(std::chrono::duration<double>(clock_type::now() - start).count());
            }

            // Update our output state
            auto incrementalVM = extractModel(partialModel);
            auto integratedVM = extractModel(integratedModel);
            dense_mapping::VolumetricTimeSeriesEntry vtse;
            vtse.set_allocated_delta_model(incrementalVM);
            vtse.set_allocated_integrated_model(integratedVM);
            vtse.mutable_hardware_time_stamp()->set_nanos(sample->hardwaretimestamp().nanos());
            vtse.mutable_system_time_stamp()->set_nanos(sample->systemtimestamp().nanos());

            output.series.mutable_sequence()->insert(
                google::protobuf::MapPair<uint64_t, dense_mapping::VolumetricTimeSeriesEntry>(++frameCount, vtse));

            // Track how long it takes to merge into the integrated model (last thing we do on the iteration)
            {
                auto start = clock_type::now();
                integratedModel.merge(partialModel);
                output.mergeStatistics.update(std::chrono::duration<double>(clock_type::now() - start).count());
            }
        } catch (const std::exception& e) {
            LOG(WARNING) << "WORKER THREAD[" << std::this_thread::get_id() << "] FAILED TO PROCESS FRAME WITH HARDWARE TIMESTAMP ["
                         << sample->hardwaretimestamp().nanos() << "]: Caught exception [" << typeid(e).name() << "] - [" << e.what()
                         << "]; continuing processing";
        }
    }

    LOG(INFO) << "WORKER THREAD[" << std::this_thread::get_id() << "] FINISHED; FULFILLING PROMISE";
    result.set_value(output);
}

void processPointClouds(const std::string& fileName,
    std::map<std::string, std::shared_ptr<core::BoundedProducerConsumerBuffer<std::shared_ptr<const hal::CameraSample> > > >&
        pointBuffers) {
    LOG(INFO) << "Loading point clouds from [" << fileName << "]";
    std::ifstream in(fileName.c_str(), std::ios::in | std::ios::binary);
    serialization::ProtobufReader reader(&in);

    auto sample = std::make_shared<hal::CameraSample>();

    while (reader.readNext(*sample)) {
        LOG(INFO) << "Sanity checking point cloud with hardware timestamp [" << sample->hardwaretimestamp().nanos() << "] @ ["
                  << sample.get() << "]";
        try {
            sanityCheckImage(*sample);

            LOG(INFO) << "Propagating point cloud for processing";
            for (auto& queue : pointBuffers) {
                queue.second->enqueue(sample);
            }

            sample = std::make_shared<hal::CameraSample>();
        } catch (const std::exception& e) {
            LOG(ERROR) << "Point cloud with hardware timestamp [" << sample->hardwaretimestamp().nanos() << "] failed sanity checks: ["
                       << e.what() << "]; skipping.";
        }
    }
}
}

int main(int argc, char** argv) {
    FLAGS_logtostderr = true;

    gflags::SetUsageMessage("Process x, y, z data and vehicle telemetry from the simulator and generate a dense map");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    logCommandLineParameters();

    // Setup buffers that are shared between the point input thread and the threads doing the actual processing
    LOG(INFO) << "Preparing point cloud buffers";
    constexpr size_t pointBufferCapacity = 10;
    std::map<std::string, std::shared_ptr<core::BoundedProducerConsumerBuffer<std::shared_ptr<const hal::CameraSample> > > > pointBuffers;
    pointBuffers.insert(std::make_pair(std::string("groundtruth"),
        std::make_shared<core::BoundedProducerConsumerBuffer<std::shared_ptr<const hal::CameraSample> > >(pointBufferCapacity)));
    pointBuffers.insert(std::make_pair(std::string("odometry"),
        std::make_shared<core::BoundedProducerConsumerBuffer<std::shared_ptr<const hal::CameraSample> > >(pointBufferCapacity)));

    // Setup pose sources
    LOG(INFO) << "Preparing pose sources";
    std::map<std::string, std::map<uint64_t, Sophus::SE3<scalar_type> > > poses;
    poses["groundtruth"] = loadPosesFromGroundTruth(FLAGS_unityGroundTruthPoseFile);
    poses["odometry"] = loadPosesFromOdometry(FLAGS_telemetryPoseFile);

    // Fire up all the worker threads
    LOG(INFO) << "Preparing processing threads";
    std::map<std::string, std::shared_ptr<std::thread> > workerThreads;
    std::map<std::string, std::future<VolumetricTimeSeriesState> > workerResults;

    for (auto& pose : poses) {
        auto description = pose.first;
        auto poseSource = &pose.second;
        auto buffer = pointBuffers.find(description)->second;

        std::promise<VolumetricTimeSeriesState> promise;
        workerResults.emplace(std::make_pair(description, promise.get_future()));

        // Be *very* careful with the captures in this lambda -- it is essential that promise is moved and that buffer /
        // pose source are copied, otherwise you're setting yourself for all sorts of "fun" undefined behavior
        workerThreads[pose.first] = std::make_shared<std::thread>(
            [ promise{ std::move(promise) }, buffer, poseSource ]() mutable { buildTimeSeries(*buffer, *poseSource, std::move(promise)); });
    }

    // Process images
    LOG(INFO) << "Processing point cloud data";
    processPointClouds(FLAGS_cameraDataFile, pointBuffers);

    // Let everyone know there's no input and we'd like the results
    LOG(INFO) << "Final frame dequeued; sending shutdown signals to shared queues";
    for (auto& queue : pointBuffers) {
        queue.second->shutdown();
    }

    LOG(INFO) << "Merging results from worker threads";
    dense_mapping::VolumetricTimeSeriesDataset dataset;
    SummaryStatistics<scalar_type> combinedInsertStatistics;
    SummaryStatistics<scalar_type> combinedMergeStatistics;

    for (auto& future : workerResults) {
        auto result = future.second.get();
        combinedInsertStatistics.merge(result.insertStatistics);
        combinedMergeStatistics.merge(result.mergeStatistics);
        (*dataset.mutable_timeseries())[future.first] = result.series;
    }

    LOG(INFO) << "Joining worker threads";
    for (auto& thread : workerThreads) {
        thread.second->join();
    }

    LOG(INFO) << "Insert timing statistics:\n" << combinedInsertStatistics << "\nMerge timing statistics:\n" << combinedMergeStatistics;

    LOG(INFO) << "Writing final data set to [" << FLAGS_outputFile << "]";
    std::ofstream out(FLAGS_outputFile.c_str(), std::ios::binary);
    google::protobuf::io::OstreamOutputStream oos(&out);
    google::protobuf::io::CodedOutputStream cos(&oos);
    dataset.SerializeToCodedStream(&cos);

    return 0;
}