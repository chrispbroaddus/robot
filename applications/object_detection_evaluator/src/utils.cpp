#include "applications/object_detection_evaluator/include/utils.h"
#include "packages/machine_learning/include/detection_utils.h"

namespace benchmark {

EvaluatedDetection evaluateDetectionResults(
    const std::vector<std::pair<perception::CameraAlignedBoxDetection, perception::FrameAnnotation> >& queryAnnotationPair,
    float iouRatioThreshold) {

    EvaluatedDetection evals;

    for (size_t i = 0; i < queryAnnotationPair.size(); i++) {

        auto evalFrame = evals.add_frames();

        auto queryFrame = queryAnnotationPair[i].first;
        auto annoFrame = queryAnnotationPair[i].second;

        evalFrame->set_num_ground_truth_boxes(annoFrame.boxes_2d_size());

        std::vector<bool> groundTruthFound(annoFrame.boxes_2d_size(), false);
        for (int j = 0; j < queryFrame.bounding_boxes_size(); j++) {

            auto evalBox = evalFrame->add_boxes();
            evalBox->mutable_category()->ParseFromString(queryFrame.bounding_boxes(j).category().SerializeAsString());
            evalBox->set_true_positive(false);

            for (int k = 0; k < annoFrame.boxes_2d_size(); k++) {
                if (!groundTruthFound[k]
                    && object_detection::compareBoundingBoxPairClass(queryFrame.bounding_boxes(j), annoFrame.boxes_2d(k))
                    && object_detection::computeBoundingBoxPairIouRatio(queryFrame.bounding_boxes(j), annoFrame.boxes_2d(k))
                        >= iouRatioThreshold) {
                    evalBox->set_true_positive(true);
                }
            }
        }
    }
    return evals;
}

std::vector<benchmark::PrecisionRecallPoint> calculatePRCurve(const EvaluatedDetection& evals, double recallIntervalSize) {

    // Concatenated global (i.e. not frame-wise) scores
    std::vector<int> globalScores;

    // Local index to each detection candidate within frame
    std::vector<int> localIndices;

    // Backward-mapping to the frame index to queryAnnotationPair from the global index
    std::vector<int> frameIndexFromGlobalDetCandId;

    // Global counter for the number of ground truth bounding boxes
    int numGlobalGroundTruthBoxes = 0;

    for (int i = 0; i < evals.frames_size(); i++) {

        // Update global scores
        std::transform(evals.frames(i).boxes().begin(), evals.frames(i).boxes().end(), std::back_inserter(globalScores),
            [](const EvaluatedDetectionBoundingBox& bbox) { return bbox.category().confidence(); });

        // Update local indices
        std::vector<int> localIndicesPerFrame(evals.frames(i).boxes_size());
        std::iota(localIndicesPerFrame.begin(), localIndicesPerFrame.end(), 0);
        localIndices.insert(localIndices.end(), localIndicesPerFrame.begin(), localIndicesPerFrame.end());

        // Update pairIds
        std::vector<int> frameIndexFromLocalDetCandId(evals.frames(i).boxes_size(), i);
        frameIndexFromGlobalDetCandId.insert(
            frameIndexFromGlobalDetCandId.end(), frameIndexFromLocalDetCandId.begin(), frameIndexFromLocalDetCandId.end());

        // Update global ground truth bbox counter
        numGlobalGroundTruthBoxes += evals.frames(i).num_ground_truth_boxes();
    }

    std::vector<int> globalIndices(globalScores.size());
    std::iota(globalIndices.begin(), globalIndices.end(), 0);

    std::sort(std::begin(globalIndices), std::end(globalIndices), [&](int i1, int i2) { return globalScores[i1] > globalScores[i2]; });

    std::vector<benchmark::PrecisionRecallPoint> prPoints;
    benchmark::PrecisionRecallPoint pointBegin;
    pointBegin.set_precision(1);
    pointBegin.set_recall(0);
    prPoints.push_back(pointBegin);

    int countTP = 0;
    for (size_t i = 0; i < globalIndices.size(); i++) {
        int frameId = frameIndexFromGlobalDetCandId[i];

        if (evals.frames(frameId).boxes(localIndices[i]).true_positive()) {
            countTP++;
            benchmark::PrecisionRecallPoint prPoint;
            prPoint.set_precision((double)countTP / (i + 1));
            prPoint.set_recall((double)countTP / numGlobalGroundTruthBoxes);
            const double previousRecall = prPoints.back().recall();
            if (prPoint.recall() >= previousRecall + recallIntervalSize) {
                prPoint.set_score_threshold(globalScores[globalIndices[i]]);
                prPoints.push_back(prPoint);
            }
        }
    }

    benchmark::PrecisionRecallPoint pointEnd;
    pointEnd.set_precision(0);
    pointEnd.set_recall(1);
    prPoints.push_back(pointEnd);

    return prPoints;
}

double computePRCurveAuc(const std::vector<benchmark::PrecisionRecallPoint>& prPoints) {
    CHECK(prPoints.size() >= 2);
    CHECK_EQ(prPoints.front().recall(), 0);
    CHECK_EQ(prPoints.front().precision(), 1);
    CHECK_EQ(prPoints.back().recall(), 1);
    CHECK_EQ(prPoints.back().precision(), 0);
    double area = 0;
    for (size_t i = 1; i < prPoints.size(); i++) {
        CHECK(prPoints[i].recall() >= prPoints[i - 1].recall());
        area += prPoints[i].precision() * (prPoints[i].recall() - prPoints[i - 1].recall());
    }
    return area;
}
}
