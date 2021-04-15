#include "packages/machine_learning/include/detection_utils.h"
#include "glog/logging.h"

bool object_detection::compareBoundingBoxPairClass(const perception::ObjectBoundingBox& bboxA, const perception::ObjectBoundingBox& bboxB) {
    return bboxA.category().type() == bboxB.category().type();
}

float object_detection::computeBoundingBoxPairIouRatio(
    const perception::ObjectBoundingBox& bboxA, const perception::ObjectBoundingBox& bboxB) {

    if (bboxA.extents_x() <= 0 || bboxA.extents_y() <= 0 || bboxB.extents_x() <= 0 || bboxB.extents_y() <= 0) {
        LOG(WARNING) << __PRETTY_FUNCTION__ << " ... Bounding box sizes should be positive values.";
        return 0;
    }

    const float topleftIntersectionX = std::max(bboxA.top_left_x(), bboxB.top_left_x());
    const float topleftIntersectionY = std::max(bboxA.top_left_y(), bboxB.top_left_y());
    const float bottomRightIntersectionX = std::min(bboxA.top_left_x() + bboxA.extents_x() - 1, bboxB.top_left_x() + bboxB.extents_x() - 1);
    const float bottomRightIntersectionY = std::min(bboxA.top_left_y() + bboxA.extents_y() - 1, bboxB.top_left_y() + bboxB.extents_y() - 1);
    const float areaIntersection
        = (bottomRightIntersectionX - topleftIntersectionX + 1) * (bottomRightIntersectionY - topleftIntersectionY + 1);

    if (bottomRightIntersectionX <= topleftIntersectionX - 1 || bottomRightIntersectionY <= topleftIntersectionY - 1) {
        return 0;
    }

    const float ax = bboxA.extents_x();
    const float ay = bboxA.extents_y();
    const float areaA = ax * ay;
    const float bx = bboxB.extents_x();
    const float by = bboxB.extents_y();
    const float areaB = bx * by;
    const float areaUnion = areaA + areaB - areaIntersection;

    // note : the denominator (union) is guaranteed to always greater than 0,
    //        since it is checked that width & height of intersection are positive.
    return areaIntersection / areaUnion;
}
