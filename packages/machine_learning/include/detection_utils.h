#include "packages/perception/proto/detection.pb.h"

namespace object_detection {

///
/// @brief Returns true if boxes are for a same class
///
bool compareBoundingBoxPairClass(const perception::ObjectBoundingBox& bboxA, const perception::ObjectBoundingBox& bboxB);

///
/// @brief Returns the IOU (Intersection-Over-Union) ratio between two boxes.
///
float computeBoundingBoxPairIouRatio(const perception::ObjectBoundingBox& bboxA, const perception::ObjectBoundingBox& bboxB);
}
