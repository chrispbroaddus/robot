#include "glog/logging.h"
#include "gtest/gtest.h"

#include "packages/object_tracking/include/sbbc_track.h"
#include "packages/object_tracking/include/track_manager.h"

constexpr int kMaxNumFramesPerTrack = 5;

using namespace object_tracking;

using SBBCTrack = SimpleBoundingBoxContextTrack;

TEST(SimpleBoundingBoxContextTrack, instantiation) { SBBCTrack track(kMaxNumFramesPerTrack); }

TEST(TrackManager_SBBCTrack, twoConsequtiveFramesTwoObjects) {

    std::shared_ptr<SimpleBoundingBoxContextTrackingAlgorithm::MatchParams> sbbcMatchParams;
    sbbcMatchParams.reset(new SimpleBoundingBoxContextTrackingAlgorithm::MatchParams());
    sbbcMatchParams->iouThreshold = 0.5;

    TrackManager<SBBCTrack> manager(kMaxNumFramesPerTrack, sbbcMatchParams);

    // Synthesized detection to test object tracking for the first frame
    perception::Detection detectionFirstFrame;
    auto box1 = detectionFirstFrame.mutable_box_detection()->add_bounding_boxes();
    box1->mutable_category()->set_type(perception::Category_CategoryType::Category_CategoryType_PERSON);
    box1->set_top_left_x(100);
    box1->set_top_left_y(100);
    box1->set_extents_x(50);
    box1->set_extents_y(50);

    auto box2 = detectionFirstFrame.mutable_box_detection()->add_bounding_boxes();
    box2->mutable_category()->set_type(perception::Category_CategoryType::Category_CategoryType_PERSON);
    box2->set_top_left_x(200);
    box2->set_top_left_y(200);
    box2->set_extents_x(50);
    box2->set_extents_y(50);

    // dummy camera sample
    hal::CameraSample cameraSampleFirstFrame;
    object_tracking::uint8_image_t imageFirstFrame(cameraSampleFirstFrame.image().rows(), cameraSampleFirstFrame.image().cols(),
        cameraSampleFirstFrame.image().stride(), (unsigned char*)cameraSampleFirstFrame.image().data().data());
    manager.addFrame(detectionFirstFrame, imageFirstFrame, cameraSampleFirstFrame.hardwaretimestamp());

    // Synthesized detection to test object tracking for the second frame
    perception::Detection detectionSecondFrame;
    auto box3 = detectionSecondFrame.mutable_box_detection()->add_bounding_boxes();
    box3->mutable_category()->set_type(perception::Category_CategoryType::Category_CategoryType_PERSON);
    box3->set_top_left_x(105);
    box3->set_top_left_y(105);
    box3->set_extents_x(50);
    box3->set_extents_y(50);

    auto box4 = detectionSecondFrame.mutable_box_detection()->add_bounding_boxes();
    box4->mutable_category()->set_type(perception::Category_CategoryType::Category_CategoryType_PERSON);
    box4->set_top_left_x(205);
    box4->set_top_left_y(205);
    box4->set_extents_x(50);
    box4->set_extents_y(50);

    // dummy camera sample
    hal::CameraSample cameraSampleSecondFrame;
    object_tracking::uint8_image_t imageSecondFrame(cameraSampleSecondFrame.image().rows(), cameraSampleSecondFrame.image().cols(),
        cameraSampleSecondFrame.image().stride(), (unsigned char*)cameraSampleSecondFrame.image().data().data());
    manager.addFrame(detectionSecondFrame, imageSecondFrame, cameraSampleSecondFrame.hardwaretimestamp());

    EXPECT_EQ(manager.trackSize(), 2);

    // object 0
    EXPECT_EQ(detectionFirstFrame.box_detection().bounding_boxes(0).instance_id(),
        detectionSecondFrame.box_detection().bounding_boxes(0).instance_id());

    // object 1
    EXPECT_EQ(detectionFirstFrame.box_detection().bounding_boxes(1).instance_id(),
        detectionSecondFrame.box_detection().bounding_boxes(1).instance_id());

    // Ids of obj0 and obj1 should be different
    EXPECT_TRUE(detectionFirstFrame.box_detection().bounding_boxes(0).instance_id()
        != detectionFirstFrame.box_detection().bounding_boxes(1).instance_id());
}

TEST(TrackManager_SBBCTrack, twoConsequtiveFramesWrongClassificationOnSecondFrame) {

    std::shared_ptr<SimpleBoundingBoxContextTrackingAlgorithm::MatchParams> sbbcMatchParams;
    sbbcMatchParams.reset(new SimpleBoundingBoxContextTrackingAlgorithm::MatchParams());
    sbbcMatchParams->iouThreshold = 0.5;

    TrackManager<SBBCTrack> manager(kMaxNumFramesPerTrack, sbbcMatchParams);

    // Synthesized detection to test object tracking for the first frame
    perception::Detection detectionFirstFrame;
    auto box1 = detectionFirstFrame.mutable_box_detection()->add_bounding_boxes();
    box1->mutable_category()->set_type(perception::Category_CategoryType::Category_CategoryType_PERSON);
    box1->set_top_left_x(100);
    box1->set_top_left_y(100);
    box1->set_extents_x(50);
    box1->set_extents_y(50);

    // dummy camera sample
    hal::CameraSample cameraSampleFirstFrame;
    object_tracking::uint8_image_t imageFirstFrame(cameraSampleFirstFrame.image().rows(), cameraSampleFirstFrame.image().cols(),
        cameraSampleFirstFrame.image().stride(), (unsigned char*)cameraSampleFirstFrame.image().data().data());
    manager.addFrame(detectionFirstFrame, imageFirstFrame, cameraSampleFirstFrame.hardwaretimestamp());

    // Synthesized detection to test object tracking for the second frame
    perception::Detection detectionSecondFrame;
    auto box2 = detectionSecondFrame.mutable_box_detection()->add_bounding_boxes();
    box2->mutable_category()->set_type(perception::Category_CategoryType::Category_CategoryType_CAR);
    box2->set_top_left_x(100);
    box2->set_top_left_y(100);
    box2->set_extents_x(50);
    box2->set_extents_y(50);

    // dummy camera sample
    hal::CameraSample cameraSampleSecondFrame;
    object_tracking::uint8_image_t imageSecondFrame(cameraSampleSecondFrame.image().rows(), cameraSampleSecondFrame.image().cols(),
        cameraSampleSecondFrame.image().stride(), (unsigned char*)cameraSampleSecondFrame.image().data().data());
    manager.addFrame(detectionSecondFrame, imageSecondFrame, cameraSampleSecondFrame.hardwaretimestamp());

    // The first track still exist, but marked as ActiveLostState.
    // The second track is for a car.
    EXPECT_EQ(manager.trackSize(), 2);

    int numActiveAdetectedTrackSize = 0;
    for (size_t i = 0; i < manager.trackSize(); i++) {
        numActiveAdetectedTrackSize += manager.track(i)->isActiveDetectedState();
    }
    EXPECT_EQ(numActiveAdetectedTrackSize, 1);

    EXPECT_TRUE(detectionFirstFrame.box_detection().bounding_boxes(0).instance_id()
        != detectionSecondFrame.box_detection().bounding_boxes(0).instance_id());
}

TEST(TrackManager_SBBCTrack, twoConsequtiveFramesDifferentLocation) {

    std::shared_ptr<SimpleBoundingBoxContextTrackingAlgorithm::MatchParams> sbbcMatchParams;
    sbbcMatchParams.reset(new SimpleBoundingBoxContextTrackingAlgorithm::MatchParams());
    sbbcMatchParams->iouThreshold = 0.5;

    TrackManager<SBBCTrack> manager(kMaxNumFramesPerTrack, sbbcMatchParams);

    // Synthesized detection to test object tracking for the first frame
    perception::Detection detectionFirstFrame;
    auto box1 = detectionFirstFrame.mutable_box_detection()->add_bounding_boxes();
    box1->mutable_category()->set_type(perception::Category_CategoryType::Category_CategoryType_PERSON);
    box1->set_top_left_x(100);
    box1->set_top_left_y(100);
    box1->set_extents_x(50);
    box1->set_extents_y(50);

    // dummy camera sample
    hal::CameraSample cameraSampleFirstFrame;
    object_tracking::uint8_image_t imageFirstFrame(cameraSampleFirstFrame.image().rows(), cameraSampleFirstFrame.image().cols(),
        cameraSampleFirstFrame.image().stride(), (unsigned char*)cameraSampleFirstFrame.image().data().data());
    manager.addFrame(detectionFirstFrame, imageFirstFrame, cameraSampleFirstFrame.hardwaretimestamp());

    // Synthesized detection to test object tracking for the second frame
    perception::Detection detectionSecondFrame;
    auto box2 = detectionSecondFrame.mutable_box_detection()->add_bounding_boxes();
    box2->mutable_category()->set_type(perception::Category_CategoryType::Category_CategoryType_PERSON);
    box2->set_top_left_x(140);
    box2->set_top_left_y(140);
    box2->set_extents_x(50);
    box2->set_extents_y(50);

    // dummy camera sample
    hal::CameraSample cameraSampleSecondFrame;
    object_tracking::uint8_image_t imageSecondFrame(cameraSampleSecondFrame.image().rows(), cameraSampleSecondFrame.image().cols(),
        cameraSampleSecondFrame.image().stride(), (unsigned char*)cameraSampleSecondFrame.image().data().data());
    manager.addFrame(detectionSecondFrame, imageSecondFrame, cameraSampleSecondFrame.hardwaretimestamp());

    // The first track still exist, but marked as ActiveLostState.
    // The second track is for a person on a different location.
    EXPECT_EQ(manager.trackSize(), 2);

    int numActiveAdetectedTrackSize = 0;
    for (size_t i = 0; i < manager.trackSize(); i++) {
        numActiveAdetectedTrackSize += manager.track(i)->isActiveDetectedState();
    }
    EXPECT_EQ(numActiveAdetectedTrackSize, 1);

    EXPECT_TRUE(detectionFirstFrame.box_detection().bounding_boxes(0).instance_id()
        != detectionSecondFrame.box_detection().bounding_boxes(0).instance_id());
}
