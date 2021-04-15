#undef VISUALIZE_DETECTION_RESULTS

#include "glog/logging.h"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "packages/machine_learning/include/object_detector.h"
#include "packages/utilities/opencv/datatype_conversions.h"
#include "gtest/gtest.h"

constexpr char kSavedFasterRcnnModel[] = "packages/machine_learning/models/faster_rcnn_resnet101_coco_11_06_2017/saved";
constexpr char kImageFilename[] = "packages/machine_learning/test/detection_test_image.jpg";

TEST(FasterRcnn, DetectionResultSanityCheck) {
    std::unique_ptr<ml::ObjectDetector> objectDetector;
    objectDetector.reset(new ml::ObjectDetector(kSavedFasterRcnnModel, 0.8, ml::CocoLabelToZippyLabelConverter()));

#ifdef VISUALIZE_DETECTION_RESULTS
    std::string outPath = "/tmp/detection_test_image_results.jpg";
#endif

    cv::Mat image = cv::imread(kImageFilename);

    hal::CameraSample cameraSample;
    OpenCVUtility::ocvMatToCameraSample(image, cameraSample);

    auto detection = objectDetector->detect(cameraSample);

    LOG(INFO) << "The number of detections : " << detection.box_detection().bounding_boxes_size();

    EXPECT_EQ(detection.box_detection().bounding_boxes_size(), 3);

    for (int i = 0; i < std::min(detection.box_detection().bounding_boxes_size(), 3); i++) {
        EXPECT_EQ(detection.box_detection().bounding_boxes(i).category().type(), perception::Category::PERSON);
#ifdef VISUALIZE_DETECTION_RESULTS
        cv::Point tl(detection.box_detection().bounding_boxes(i).top_left_x(), detection.box_detection().bounding_boxes(i).top_left_y());
        cv::Size size(detection.box_detection().bounding_boxes(i).extents_x(), detection.box_detection().bounding_boxes(i).extents_y());
        cv::Rect rect(tl, size);
        cv::rectangle(image, rect, cv::Scalar(255, 255, 255));
#endif
    }

#ifdef VISUALIZE_DETECTION_RESULTS
    cv::imwrite(outPath, image);
#endif
}
