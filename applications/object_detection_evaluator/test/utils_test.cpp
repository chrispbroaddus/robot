#include "gtest/gtest.h"

#include "applications/object_detection_evaluator/include/utils.h"
#include "packages/machine_learning/include/detection_utils.h"

using namespace benchmark;

TEST(CompareBoundingBoxPairClass, sameClass) {
    perception::ObjectBoundingBox bboxA;
    bboxA.mutable_category()->set_type(perception::Category_CategoryType::Category_CategoryType_CAR);

    perception::ObjectBoundingBox bboxB;
    bboxB.mutable_category()->set_type(perception::Category_CategoryType::Category_CategoryType_CAR);

    EXPECT_TRUE(object_detection::compareBoundingBoxPairClass(bboxA, bboxB));
}

TEST(CompareBoundingBoxPairClass, differentClass) {
    perception::ObjectBoundingBox bboxA;
    bboxA.mutable_category()->set_type(perception::Category_CategoryType::Category_CategoryType_CAR);

    perception::ObjectBoundingBox bboxB;
    bboxB.mutable_category()->set_type(perception::Category_CategoryType::Category_CategoryType_PERSON);

    EXPECT_FALSE(object_detection::compareBoundingBoxPairClass(bboxA, bboxB));
}

TEST(CalculateBoundingBoxPairIou, noOverlap) {
    perception::ObjectBoundingBox bboxA;
    bboxA.set_top_left_x(0);
    bboxA.set_top_left_y(0);
    bboxA.set_extents_x(100);
    bboxA.set_extents_y(100);

    perception::ObjectBoundingBox bboxB;
    bboxB.set_top_left_x(150);
    bboxB.set_top_left_y(150);
    bboxB.set_extents_x(100);
    bboxB.set_extents_y(100);

    float iouRatio = object_detection::computeBoundingBoxPairIouRatio(bboxA, bboxB);
    EXPECT_FLOAT_EQ(iouRatio, 0);
}

TEST(CalculateBoundingBoxPairIou, perfectOverlap) {
    perception::ObjectBoundingBox bboxA;
    bboxA.set_top_left_x(0);
    bboxA.set_top_left_y(0);
    bboxA.set_extents_x(100);
    bboxA.set_extents_y(100);

    perception::ObjectBoundingBox bboxB;
    bboxB.set_top_left_x(0);
    bboxB.set_top_left_y(0);
    bboxB.set_extents_x(100);
    bboxB.set_extents_y(100);

    float iouRatio = object_detection::computeBoundingBoxPairIouRatio(bboxA, bboxB);
    EXPECT_FLOAT_EQ(iouRatio, 1);
}

TEST(CalculateBoundingBoxPairIou, smallOverlap) {
    perception::ObjectBoundingBox bboxA;
    bboxA.set_top_left_x(0);
    bboxA.set_top_left_y(0);
    bboxA.set_extents_x(3);
    bboxA.set_extents_y(3);

    perception::ObjectBoundingBox bboxB;
    bboxB.set_top_left_x(2);
    bboxB.set_top_left_y(2);
    bboxB.set_extents_x(3);
    bboxB.set_extents_y(3);

    float iouRatio = object_detection::computeBoundingBoxPairIouRatio(bboxA, bboxB);
    // areaIntersection : 1, areaUnion : 18-1=17
    EXPECT_FLOAT_EQ(iouRatio, 1.f / (18 - 1));
}

///
/// Bbox B has a zero size, hence iouRatio should be zero.
///
TEST(CalculateBoundingBoxPairIou, zeroSizedBox) {
    perception::ObjectBoundingBox bboxA;
    bboxA.set_top_left_x(0);
    bboxA.set_top_left_y(0);
    bboxA.set_extents_x(10);
    bboxA.set_extents_y(10);

    perception::ObjectBoundingBox bboxB;
    bboxB.set_top_left_x(5);
    bboxB.set_top_left_y(5);
    bboxB.set_extents_x(0);
    bboxB.set_extents_y(0);

    float iouRatio = object_detection::computeBoundingBoxPairIouRatio(bboxA, bboxB);
    EXPECT_FLOAT_EQ(iouRatio, 0);
}

///
/// Bbox B has an invalid bbox size, hence iouRatio should be zero.
///
TEST(CalculateBoundingBoxPairIou, negativeSizedBox) {
    perception::ObjectBoundingBox bboxA;
    bboxA.set_top_left_x(0);
    bboxA.set_top_left_y(0);
    bboxA.set_extents_x(10);
    bboxA.set_extents_y(10);

    perception::ObjectBoundingBox bboxB;
    bboxB.set_top_left_x(5);
    bboxB.set_top_left_y(5);
    bboxB.set_extents_x(-5);
    bboxB.set_extents_y(-5);

    float iouRatio = object_detection::computeBoundingBoxPairIouRatio(bboxA, bboxB);
    EXPECT_FLOAT_EQ(iouRatio, 0);
}

TEST(CalculatePRCurve, oracleDetector) {
    perception::ObjectBoundingBox bboxA;
    bboxA.set_top_left_x(0);
    bboxA.set_top_left_y(0);
    bboxA.set_extents_x(10);
    bboxA.set_extents_y(10);

    perception::ObjectBoundingBox bboxB;
    bboxB.set_top_left_x(20);
    bboxB.set_top_left_y(20);
    bboxB.set_extents_x(10);
    bboxB.set_extents_y(10);

    perception::CameraAlignedBoxDetection query;
    auto queryBboxA = query.add_bounding_boxes();
    queryBboxA->ParseFromString(bboxA.SerializeAsString());
    auto queryBboxB = query.add_bounding_boxes();
    queryBboxB->ParseFromString(bboxB.SerializeAsString());

    perception::FrameAnnotation groundTruth;
    auto gtBoxA = groundTruth.add_boxes_2d();
    gtBoxA->ParseFromString(bboxA.SerializeAsString());
    auto gtBoxB = groundTruth.add_boxes_2d();
    gtBoxB->ParseFromString(bboxB.SerializeAsString());

    auto queryRefPair = std::make_pair(query, groundTruth);
    std::vector<std::pair<perception::CameraAlignedBoxDetection, perception::FrameAnnotation> > multiFrameQueryRefPair;
    multiFrameQueryRefPair.push_back(queryRefPair);
    const auto evals = evaluateDetectionResults(multiFrameQueryRefPair, 0.5);
    EXPECT_EQ(evals.frames_size(), 1);
    EXPECT_EQ(evals.frames(0).num_ground_truth_boxes(), 2);
    EXPECT_EQ(evals.frames(0).boxes_size(), 2);
    EXPECT_EQ(evals.frames(0).boxes(0).true_positive(), true);
    EXPECT_EQ(evals.frames(0).boxes(1).true_positive(), true);

    const auto prPoints = calculatePRCurve(evals);

    EXPECT_EQ(prPoints.size(), 4);
    const float auc = computePRCurveAuc(prPoints);
    EXPECT_FLOAT_EQ(auc, 1.0);
}

TEST(CalculatePRCurve, totallyWrongDetector) {
    perception::ObjectBoundingBox bboxA;
    bboxA.set_top_left_x(0);
    bboxA.set_top_left_y(0);
    bboxA.set_extents_x(10);
    bboxA.set_extents_y(10);

    perception::ObjectBoundingBox bboxB;
    bboxB.set_top_left_x(20);
    bboxB.set_top_left_y(20);
    bboxB.set_extents_x(10);
    bboxB.set_extents_y(10);

    perception::CameraAlignedBoxDetection query;
    auto queryBboxA = query.add_bounding_boxes();
    queryBboxA->ParseFromString(bboxA.SerializeAsString());
    auto queryBboxB = query.add_bounding_boxes();
    queryBboxB->ParseFromString(bboxB.SerializeAsString());

    perception::ObjectBoundingBox bboxC;
    bboxC.set_top_left_x(100);
    bboxC.set_top_left_y(100);
    bboxC.set_extents_x(10);
    bboxC.set_extents_y(10);

    perception::ObjectBoundingBox bboxD;
    bboxD.set_top_left_x(200);
    bboxD.set_top_left_y(200);
    bboxD.set_extents_x(10);
    bboxD.set_extents_y(10);

    perception::FrameAnnotation groundTruth;
    auto gtBoxA = groundTruth.add_boxes_2d();
    gtBoxA->ParseFromString(bboxC.SerializeAsString());
    auto gtBoxB = groundTruth.add_boxes_2d();
    gtBoxB->ParseFromString(bboxD.SerializeAsString());

    auto queryRefPair = std::make_pair(query, groundTruth);
    std::vector<std::pair<perception::CameraAlignedBoxDetection, perception::FrameAnnotation> > multiFrameQueryRefPair;
    multiFrameQueryRefPair.push_back(queryRefPair);
    const auto evals = evaluateDetectionResults(multiFrameQueryRefPair, 0.5);
    EXPECT_EQ(evals.frames_size(), 1);
    EXPECT_EQ(evals.frames(0).num_ground_truth_boxes(), 2);
    EXPECT_EQ(evals.frames(0).boxes_size(), 2);
    EXPECT_EQ(evals.frames(0).boxes(0).true_positive(), false);
    EXPECT_EQ(evals.frames(0).boxes(1).true_positive(), false);

    const auto prPoints = calculatePRCurve(evals);

    const float auc = computePRCurveAuc(prPoints);
    EXPECT_FLOAT_EQ(auc, 0);
}

TEST(CalculatePRCurve, correctLocationButWrongClassDetector) {
    perception::ObjectBoundingBox bboxA;
    bboxA.mutable_category()->set_type(perception::Category_CategoryType::Category_CategoryType_CAR);
    bboxA.set_top_left_x(0);
    bboxA.set_top_left_y(0);
    bboxA.set_extents_x(10);
    bboxA.set_extents_y(10);

    perception::ObjectBoundingBox bboxB;
    bboxB.mutable_category()->set_type(perception::Category_CategoryType::Category_CategoryType_PERSON);
    bboxB.set_top_left_x(20);
    bboxB.set_top_left_y(20);
    bboxB.set_extents_x(10);
    bboxB.set_extents_y(10);

    perception::CameraAlignedBoxDetection query;
    auto queryBboxA = query.add_bounding_boxes();
    queryBboxA->ParseFromString(bboxA.SerializeAsString());
    auto queryBboxB = query.add_bounding_boxes();
    queryBboxB->ParseFromString(bboxB.SerializeAsString());

    perception::ObjectBoundingBox bboxC;
    bboxC.mutable_category()->set_type(perception::Category_CategoryType::Category_CategoryType_PERSON);
    bboxC.set_top_left_x(0);
    bboxC.set_top_left_y(0);
    bboxC.set_extents_x(10);
    bboxC.set_extents_y(10);

    perception::ObjectBoundingBox bboxD;
    bboxD.mutable_category()->set_type(perception::Category_CategoryType::Category_CategoryType_CAR);
    bboxD.set_top_left_x(20);
    bboxD.set_top_left_y(20);
    bboxD.set_extents_x(10);
    bboxD.set_extents_y(10);

    perception::FrameAnnotation groundTruth;
    auto gtBoxA = groundTruth.add_boxes_2d();
    gtBoxA->ParseFromString(bboxC.SerializeAsString());
    auto gtBoxB = groundTruth.add_boxes_2d();
    gtBoxB->ParseFromString(bboxD.SerializeAsString());

    auto queryRefPair = std::make_pair(query, groundTruth);
    std::vector<std::pair<perception::CameraAlignedBoxDetection, perception::FrameAnnotation> > multiFrameQueryRefPair;
    multiFrameQueryRefPair.push_back(queryRefPair);
    const auto evals = evaluateDetectionResults(multiFrameQueryRefPair, 0.5);
    EXPECT_EQ(evals.frames_size(), 1);
    EXPECT_EQ(evals.frames(0).num_ground_truth_boxes(), 2);
    EXPECT_EQ(evals.frames(0).boxes_size(), 2);
    EXPECT_EQ(evals.frames(0).boxes(0).true_positive(), false);
    EXPECT_EQ(evals.frames(0).boxes(1).true_positive(), false);

    const auto prPoints = calculatePRCurve(evals);

    const float auc = computePRCurveAuc(prPoints);
    EXPECT_FLOAT_EQ(auc, 0);
}