#include "packages/machine_learning/include/object_detector.h"
#include "glog/logging.h"

namespace {
constexpr int kMaxNumDetection = 300;
void DummyDeallocator(void*, size_t, void*) {}
}

namespace ml {

ObjectDetector::ObjectDetector(const std::string& savedModelFileName, const float detectionScoreThreshold,
    std::function<perception::Category_CategoryType(const int)> labelConverter)
    : m_graphWrapper(savedModelFileName)
    , m_detectionScoreThreshold(detectionScoreThreshold)
    , m_labelConverter(labelConverter) {
    // note : "detection_masks" is not being used.
    TFGraphWrapper::Node outputNode1("num_detections", 2, { 1, 1 });
    TFGraphWrapper::Node outputNode2("detection_boxes", 3, { 1, kMaxNumDetection, 4 }); // ymin,xmin,ymax,xmax
    TFGraphWrapper::Node outputNode3("detection_scores", 2, { 1, kMaxNumDetection });
    TFGraphWrapper::Node outputNode4("detection_classes", 2, { 1, kMaxNumDetection });
    m_outputFeatures.push_back(outputNode1);
    m_outputFeatures.push_back(outputNode2);
    m_outputFeatures.push_back(outputNode3);
    m_outputFeatures.push_back(outputNode4);
}

perception::Detection ObjectDetector::detect(hal::CameraSample& cameraSample) {

    std::vector<TFGraphWrapper::Node> inputFeatures;

    TFGraphWrapper::Node inputNode("image_tensor", 4, { 1, cameraSample.image().rows(), cameraSample.image().cols(), 3 });
    inputFeatures.push_back(inputNode);

    // Create vectors to store graph input operations and input tensors
    std::vector<TF_Tensor*> inputValues;

    // Create variables to store the size of the input and output variables
    const int numBytesIn = cameraSample.image().rows() * cameraSample.image().cols() * 3 * sizeof(uint8_t);

    // Create the input tensor using the dimension (in_dims) and size (num_bytes_in)
    // variables created earlier
    TF_Tensor* inputImageTensor = TF_NewTensor(TF_UINT8, inputFeatures[0].size.data(), inputFeatures[0].dimension,
        (void*)cameraSample.image().data().data(), numBytesIn, &DummyDeallocator, 0);

    inputValues.push_back(inputImageTensor);

    ////////////////////////////
    /// Main detection
    ////////////////////////////
    std::vector<TF_Tensor*> outputValues;
    m_graphWrapper.run(outputValues, m_outputFeatures, inputValues, inputFeatures);

    TF_DeleteTensor(inputImageTensor);

    ////////////////////////////
    /// Parse the results
    ////////////////////////////
    float* numDetectionPtr = static_cast<float*>(TF_TensorData(outputValues[0]));
    float* detectionBoxes = static_cast<float*>(TF_TensorData(outputValues[1]));
    float* detectionScores = static_cast<float*>(TF_TensorData(outputValues[2]));
    float* detectionClasses = static_cast<float*>(TF_TensorData(outputValues[3]));

    perception::Detection detection;
    perception::CameraAlignedBoxDetection* cameraAlignedBoxDetection = new perception::CameraAlignedBoxDetection();
    CHECK_NOTNULL(cameraAlignedBoxDetection);

    perception::DeviceMetadata* deviceMetadata = new perception::DeviceMetadata();
    CHECK_NOTNULL(deviceMetadata);
    deviceMetadata->mutable_device()->ParseFromString(cameraSample.SerializeAsString());
    deviceMetadata->mutable_sensor_time()->ParseFromString(cameraSample.systemtimestamp().SerializeAsString());

    perception::CameraDeviceMetadata* cameraDeviceMetadata = new perception::CameraDeviceMetadata();
    CHECK_NOTNULL(cameraDeviceMetadata);
    cameraDeviceMetadata->set_allocated_device_metadata(deviceMetadata);
    cameraDeviceMetadata->set_image_height_pixels(cameraSample.image().rows());
    cameraDeviceMetadata->set_image_width_pixels(cameraSample.image().cols());

    cameraAlignedBoxDetection->set_allocated_camera_device_metadata(cameraDeviceMetadata);

    const int numDetection = std::min((int)numDetectionPtr[0], kMaxNumDetection);
    for (int i = 0; i < numDetection; i++) {

        if (detectionScores[i] < m_detectionScoreThreshold) {
            continue;
        }

        auto boundingBox = cameraAlignedBoxDetection->add_bounding_boxes();

        perception::Category* detectedClass = new perception::Category();
        CHECK_NOTNULL(detectedClass);

        detectedClass->set_type(m_labelConverter((int)detectionClasses[i]));
        detectedClass->set_confidence(detectionScores[i]);
        boundingBox->set_allocated_category(detectedClass);

        // order : top-left-y, top-left-x, bottom-right-y, bottom-right-x
        const float bb[] = { detectionBoxes[4 * i], detectionBoxes[4 * i + 1], detectionBoxes[4 * i + 2], detectionBoxes[4 * i + 3] };
        boundingBox->set_top_left_y(bb[0] * cameraSample.image().rows());
        boundingBox->set_top_left_x(bb[1] * cameraSample.image().cols());
        boundingBox->set_extents_y((bb[2] - bb[0]) * cameraSample.image().rows());
        boundingBox->set_extents_x((bb[3] - bb[1]) * cameraSample.image().cols());
    }
    detection.set_allocated_box_detection(cameraAlignedBoxDetection);
    return detection;
}
}