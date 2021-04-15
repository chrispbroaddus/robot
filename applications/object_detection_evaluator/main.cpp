#include "gflags/gflags.h"
#include "glog/logging.h"
#include "google/protobuf/util/json_util.h"

#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include "applications/object_detection_evaluator/include/utils.h"
#include "applications/object_detection_evaluator/proto/metrics.pb.h"
#include "packages/machine_learning/include/object_detector.h"
#include "packages/perception/proto/dataset.pb.h"
#include "packages/utilities/opencv/datatype_conversions.h"

#include <chrono>
#include <fstream>
#include <string>

DEFINE_string(dataset, "", "The dataset to run evaluation.");
DEFINE_string(annotation, "", "An absolute path to the annotation json file, formatted in perception::Dataset.");
DEFINE_string(model, "", "Detection model.");
DEFINE_double(score_threshold, 0.8, "Detection score threshold.");
DEFINE_double(iou_threshold, 0.5, "Intersection-over-union threshold.");
DEFINE_string(output, "", "Output, summary of the evaluation.");

void checkInputFlags();

///
/// Load object detection model & dataset and returns the summary of evalution in benchmark::DetectionPerformance format.
///
int main(int argc, char** argv) {

    google::InstallFailureSignalHandler();
    google::InitGoogleLogging(argv[0]);

    gflags::ParseCommandLineFlags(&argc, &argv, false);
    FLAGS_logtostderr = true;

    checkInputFlags();

    perception::Dataset dataset;

    std::ifstream jsonStream = std::ifstream(FLAGS_annotation);
    if (!jsonStream) {
        throw std::runtime_error("The json stream data is invalid.");
    }

    std::stringstream buffer;
    buffer << jsonStream.rdbuf();
    auto status = google::protobuf::util::JsonStringToMessage(buffer.str(), &dataset);

    std::unique_ptr<ml::ObjectDetector> objectDetector;
    objectDetector.reset(new ml::ObjectDetector(FLAGS_model, FLAGS_score_threshold, ml::CocoLabelToZippyLabelConverter()));

    std::vector<std::pair<perception::CameraAlignedBoxDetection, perception::FrameAnnotation> > multiFrameQueryRefPair;

    CHECK(dataset.devices_size() == 1) << "The current version of the evaluation pipeline supports only 1 sensor.";

    const auto& device = dataset.devices(0);

    std::chrono::duration<double, std::milli> totalDurationInMs(0);

    for (int j = 0; j < device.frames_size(); j++) {

        const auto& frame = device.frames(j);

        std::string imagePath = FLAGS_dataset + "/" + frame.path() + frame.filename();
        cv::Mat image = cv::imread(imagePath);

        hal::CameraSample cameraSample;
        OpenCVUtility::ocvMatToCameraSample(image, cameraSample);

        auto t1 = std::chrono::high_resolution_clock::now();
        auto detection = objectDetector->detect(cameraSample);
        auto t2 = std::chrono::high_resolution_clock::now();

        totalDurationInMs += (t2 - t1);

        LOG(INFO) << " ... Detected # objs: " << detection.box_detection().bounding_boxes_size();

        auto queryRefPair = std::make_pair(detection.box_detection(), frame);
        multiFrameQueryRefPair.push_back(queryRefPair);
    }

    const auto evals = benchmark::evaluateDetectionResults(multiFrameQueryRefPair, FLAGS_iou_threshold);
    const auto prPoints = benchmark::calculatePRCurve(evals);
    const auto auc = benchmark::computePRCurveAuc(prPoints);
    LOG(INFO) << " ... PR-Curve AuC : " << auc;

    benchmark::DetectionPerformance performance;
    performance.set_average_nanosec_per_image((float)totalDurationInMs.count() / device.frames_size() * 1e6f);
    LOG(INFO) << " ... Average time per image : " << (float)totalDurationInMs.count() / device.frames_size() << " (ms)";

    performance.set_num_images(device.frames_size());
    performance.set_score_threshold(FLAGS_score_threshold);
    performance.set_iou_threshold(FLAGS_iou_threshold);
    performance.set_pr_auc(auc);
    performance.mutable_dataset()->ParseFromString(dataset.SerializeAsString());
    for (const auto& rawPrPoint : prPoints) {
        auto prPoint = performance.add_pr_points();
        prPoint->set_precision(rawPrPoint.precision());
        prPoint->set_recall(rawPrPoint.recall());
        prPoint->set_score_threshold(rawPrPoint.score_threshold());
    }

    std::string jsonString;
    google::protobuf::util::MessageToJsonString(performance, &jsonString);

    std::ofstream output;
    output.open(FLAGS_output);
    output << jsonString;
    output.close();
    return 0;
}

void checkInputFlags() {
    int status = 0;
    if (FLAGS_dataset.empty()) {
        LOG(WARNING) << "The option -dataset is required.";
        status = -1;
    }

    if (FLAGS_annotation.empty()) {
        LOG(WARNING) << "The option -annotation is required.";
        status = -2;
    }

    if (FLAGS_model.empty()) {
        LOG(WARNING) << "The option -model is required.";
        status = -3;
    }
    if (status != 0) {
        throw std::runtime_error("Any input flag is missing.");
    }
}
