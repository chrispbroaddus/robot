#include "applications/object_detection_evaluator/proto/metrics.pb.h"
#include "glog/logging.h"
#include "packages/perception/proto/detection.pb.h"

#include <algorithm>

namespace benchmark {

///
/// @brief Evaluate the detection results and evaluate detected bounding box wrt ground truth
///
EvaluatedDetection evaluateDetectionResults(
    const std::vector<std::pair<perception::CameraAlignedBoxDetection, perception::FrameAnnotation> >& queryAnnotationPair,
    float iouRatioThreshold);

///
/// @brief Returns points for the PR curves, sorted by recall rate in ascending order
///
std::vector<benchmark::PrecisionRecallPoint> calculatePRCurve(const EvaluatedDetection& evals, double recallIntervalSize);

inline std::vector<benchmark::PrecisionRecallPoint> calculatePRCurve(const EvaluatedDetection& evals) {
    return calculatePRCurve(evals, 0.01);
}

///
/// @brief Calculate area-under-curve of the PR curve
///
double computePRCurveAuc(const std::vector<benchmark::PrecisionRecallPoint>& prPoints);
}
