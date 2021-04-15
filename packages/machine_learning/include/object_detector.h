#pragma once

#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/machine_learning/include/graph_wrapper.h"
#include "packages/perception/proto/detection.pb.h"

#include <functional>
#include <string>

namespace ml {

class CocoLabelToZippyLabelConverter {
public:
    constexpr perception::Category_CategoryType operator()(const int cocoLabel) const {
        switch (cocoLabel) {
        case 1:
            return perception::Category::PERSON;

        case 2:
            return perception::Category::BICYCLE;

        case 3:
            return perception::Category::CAR;

        case 4:
            return perception::Category::MOTORBIKE;

        case 6:
            return perception::Category::BUS;

        case 8:
            return perception::Category::TRUCK;

        default:
            return perception::Category::UNKNOWN;
        }
    }
};

///
/// Object detector with the zoo model (v1.2.1) from
/// https://github.com/tensorflow/models/blob/master/object_detection/g3doc/detection_model_zoo.md
/// The model needs to be converted into Tensorflow's SaveModel using the script:
/// /script/ml/convert_pb_to_TFSavedModel.py
///
class ObjectDetector {
public:
    ObjectDetector(const std::string& savedModelFileName, const float detectionScoreThreshold,
        std::function<perception::Category_CategoryType(const int)> labelConverter);
    ObjectDetector(const TFGraphWrapper& ObjectDetector) = delete;
    ObjectDetector(const TFGraphWrapper&& ObjectDetector) = delete; // move constructor is not well defined from libtensorflow
    ObjectDetector& operator=(const TFGraphWrapper& ObjectDetector) = delete;
    ObjectDetector& operator=(const TFGraphWrapper&& ObjectDetector) = delete; // move assign op is not well defined from libtensorflow
    ~ObjectDetector() = default;

    ///
    /// @brief Detect the objects from the camera image and returns the detection results.
    ///
    perception::Detection detect(hal::CameraSample& cameraSample);

private:
    TFGraphWrapper m_graphWrapper;
    float m_detectionScoreThreshold;
    std::vector<TFGraphWrapper::Node> m_outputFeatures;
    std::function<perception::Category_CategoryType(const int)> m_labelConverter;
};
}