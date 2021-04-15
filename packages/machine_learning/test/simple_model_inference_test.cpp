#include "glog/logging.h"
#include "packages/machine_learning/include/graph_wrapper.h"
#include "gtest/gtest.h"

#include <stdio.h>

using namespace ml;

constexpr char kSavedFasterRcnnModel[] = "packages/machine_learning/models/saved_model_half_plus_two";

float runHalfPlusTwoTest(
    ml::TFGraphWrapper& graph, const std::string& inputFeatureName, const std::string& outputFeatureName, float inputValue) {

    std::vector<TFGraphWrapper::Node> outputFeatures;
    std::vector<TFGraphWrapper::Node> inputFeatures;

    TFGraphWrapper::Node inputNode("x", 1, { 1 });
    inputFeatures.push_back(inputNode);

    TFGraphWrapper::Node outputNode("y", 1, { 1 });
    outputFeatures.push_back(outputNode);

    // Create variables to store the size of the input and output variables
    const int numBytesIn = 1 * sizeof(float);

    // Create a variable containing your values, in this case the input is a 1-dimensional float
    float values[] = { inputValue };

    // Create vectors to store graph input operations and input tensors
    std::vector<TF_Output> inputs;
    std::vector<TF_Tensor*> input_values;

    // Create the input tensor using the dimension (in_dims) and size (num_bytes_in)
    // variables created earlier
    TF_Tensor* input
        = TF_NewTensor(TF_FLOAT, inputFeatures[0].size.data(), inputFeatures[0].dimension, values, numBytesIn, &ml::DummyDeallocator, 0);
    input_values.push_back(input);

    std::vector<TF_Tensor*> outputValues;
    graph.run(outputValues, outputFeatures, input_values, inputFeatures);

    float* outVals = static_cast<float*>(TF_TensorData(outputValues[0]));
    return outVals[0];
}

TEST(TensorflowBasics, LoadMultiplyGraph) {
    std::unique_ptr<ml::TFGraphWrapper> graph;
    graph.reset(new ml::TFGraphWrapper(std::string(kSavedFasterRcnnModel)));

    LOG(INFO) << "1";

    EXPECT_FLOAT_EQ(runHalfPlusTwoTest(*graph, "x", "y", 0.f), 2.f);
    EXPECT_FLOAT_EQ(runHalfPlusTwoTest(*graph, "x", "y", 1.f), 2.5f);
    EXPECT_FLOAT_EQ(runHalfPlusTwoTest(*graph, "x", "y", 10.f), 7.f);
}
