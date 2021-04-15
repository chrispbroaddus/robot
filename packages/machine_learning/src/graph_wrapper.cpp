#include "packages/machine_learning/include/graph_wrapper.h"
#include "thirdparty/tensorflow/core/protobuf/config.pb.h"

#include <vector>

#include "glog/logging.h"

///
/// Tensorflow-Graph wrapper for inference
/// Load the pre-trained model when constructed.
///
ml::TFGraphWrapper::TFGraphWrapper(const std::string& filename) {

    m_status = TF_NewStatus();

    tensorflow::GPUOptions* gpuOptions = new tensorflow::GPUOptions();
    gpuOptions->set_allow_growth(true);

    tensorflow::ConfigProto* configProto = new tensorflow::ConfigProto();
    configProto->set_allocated_gpu_options(gpuOptions);
    std::string configProtoStr = configProto->SerializeAsString();

    TF_SessionOptions* opt = TF_NewSessionOptions();
    TF_SetConfig(opt, (void*)configProtoStr.c_str(), configProtoStr.length(), m_status);

    TF_Buffer* run_options = TF_NewBufferFromString("", 0);
    TF_Buffer* metagraph = TF_NewBuffer();

    const char* tags[] = { "serve" };
    m_graph = TF_NewGraph();
    m_session = TF_LoadSessionFromSavedModel(opt, run_options, filename.c_str(), tags, 1, m_graph, metagraph, m_status);

    TF_DeleteBuffer(run_options);
    TF_DeleteSessionOptions(opt);

    m_metagraphDef.ParseFromArray(metagraph->data, metagraph->length);
    TF_DeleteBuffer(metagraph);

    CHECK_EQ(TF_OK, TF_GetCode(m_status)) << TF_Message(m_status);
}

ml::TFGraphWrapper::~TFGraphWrapper() {
    if (m_graph) {
        TF_DeleteGraph(m_graph);
    }
    if (m_session) {
        TF_CloseSession(m_session, m_status);
        TF_DeleteSession(m_session, m_status);
    }
    if (m_status) {
        TF_DeleteStatus(m_status);
    }
}

void ml::TFGraphWrapper::run(std::vector<TF_Tensor*>& output_values, const std::vector<Node>& outputFeatures,
    std::vector<TF_Tensor*>& input_values, const std::vector<Node>& inputFeatures) {

    // Set outputs
    std::vector<TF_Output> inputs;
    for (const auto& inputFeature : inputFeatures) {
        TF_Operation* input_op = TF_GraphOperationByName(m_graph, inputFeature.name.c_str());
        TF_Output input_opout = { input_op, 0 };
        inputs.push_back(input_opout);
        VLOG(2) << "TF_OperationName(input_op) : " << TF_OperationName(input_op);
    }
    CHECK_EQ(inputs.size(), input_values.size());

    // Set outputs
    std::vector<TF_Output> outputs;
    for (const auto& outputFeature : outputFeatures) {
        TF_Operation* output_op = TF_GraphOperationByName(m_graph, outputFeature.name.c_str());
        TF_Output output_opout = { output_op, 0 };
        outputs.push_back(output_opout);
        VLOG(2) << "TF_OperationName(output_op) : " << TF_OperationName(output_op);

        TF_Tensor* output_value
            = TF_AllocateTensor(TF_FLOAT, outputFeature.size.data(), outputFeature.dimension, outputFeature.dimension * sizeof(float));
        output_values.push_back(output_value);
    }
    CHECK_EQ(outputs.size(), output_values.size());

    // Call TF_SessionRun
    TF_SessionRun(m_session,
        nullptr, // run-option
        &inputs[0], &input_values[0], inputs.size(), &outputs[0], &output_values[0], outputs.size(), nullptr, 0, nullptr, m_status);
}
