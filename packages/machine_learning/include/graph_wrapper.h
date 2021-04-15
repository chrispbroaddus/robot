#pragma once

#include "thirdparty/tensorflow/core/protobuf/meta_graph.pb.h"

#include <string>
#include <tensorflow/c/c_api.h>

namespace ml {

///
/// Tensorflow-Graph wrapper for inference
/// Load the pre-trained model when constructed.
///
class TFGraphWrapper {
public:
    struct Node {
        std::string name; // The name of the tensor
        int dimension; // The dimension of the tensor, e.g., 1 for scalar, 2 for 2D matrix, ...
        std::vector<int64_t> size; // The size of the tensor, and size.size() = dimension.

        Node(const std::string& pName, const int pDimension, const std::initializer_list<int64_t>& pSize)
            : name(pName)
            , dimension(pDimension)
            , size(pSize) {}
    };

    explicit TFGraphWrapper(const std::string& filename);
    TFGraphWrapper(const TFGraphWrapper& graphWrapper) = delete;
    TFGraphWrapper(TFGraphWrapper&& graphWrapper) = delete; // move constructor is not well defined from libtensorflow
    TFGraphWrapper& operator=(const TFGraphWrapper& graphWrapper) = delete;
    TFGraphWrapper& operator=(TFGraphWrapper&& graphWrapper) = delete; // move assign op is not well defined from libtensorflow
    ~TFGraphWrapper();

    void run(std::vector<TF_Tensor*>& output_values, const std::vector<Node>& outputFeatures, std::vector<TF_Tensor*>& input_values,
        const std::vector<Node>& inputFeatureNames);

private:
    TF_Session* m_session;

    TF_Status* m_status;
    TF_Graph* m_graph;

    tensorflow::MetaGraphDef m_metagraphDef;
};
}