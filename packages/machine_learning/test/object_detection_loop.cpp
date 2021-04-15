#include "glog/logging.h"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "packages/machine_learning/include/object_detector.h"
#include "packages/utilities/opencv/datatype_conversions.h"
#include "gtest/gtest.h"

constexpr char kModel[] = "packages/machine_learning/models/ssd_mobilenet_v1_coco_11_06_2017/saved";
constexpr char kImageFilename[] = "packages/machine_learning/test/detection_test_image.jpg";

TEST(FasterRcnn, DetectionResultSanityCheck) {
    std::unique_ptr<ml::ObjectDetector> objectDetector;
    objectDetector.reset(new ml::ObjectDetector(kModel, 0.8, ml::CocoLabelToZippyLabelConverter()));

    cv::Mat image = cv::imread(kImageFilename);

    hal::CameraSample cameraSample;
    OpenCVUtility::ocvMatToCameraSample(image, cameraSample);

    for (int i = 0; i < 10000000; i++) {
        auto detection = objectDetector->detect(cameraSample);
        if (i % 100 == 0) {
            LOG(INFO) << "The number of images detected : " << i;
        }
    }
}
