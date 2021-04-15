#include "glog/logging.h"
#include "gtest/gtest.h"

#include "packages/hal/proto/coordinate_frame_id.pb.h"
#include "packages/hal/utilities/coordinate_frame_id_utils.h"

using namespace hal;

TEST(CoordinateFrameId, expect_valid_or_throw_error) {
    // direct conversion to CoordFrameId
    EXPECT_NO_THROW(getCoordinateFrameIdFromDeviceName("CoordFrameFrontLeftStereo"));
    EXPECT_NO_THROW(getCoordinateFrameIdFromDeviceName("CoordFrameFrontRightStereo"));
    EXPECT_NO_THROW(getCoordinateFrameIdFromDeviceName("CoordFrameRearLeftStereo"));
    EXPECT_NO_THROW(getCoordinateFrameIdFromDeviceName("CoordFrameRearRightStereo"));
    EXPECT_NO_THROW(getCoordinateFrameIdFromDeviceName("CoordFrameFrontFisheye"));
    EXPECT_NO_THROW(getCoordinateFrameIdFromDeviceName("CoordFrameRearFisheye"));
    EXPECT_NO_THROW(getCoordinateFrameIdFromDeviceName("CoordFrameLeftFisheye"));
    EXPECT_NO_THROW(getCoordinateFrameIdFromDeviceName("CoordFrameRightFisheye"));
    EXPECT_NO_THROW(getCoordinateFrameIdFromDeviceName("CoordFrameBaseLink"));

    // conversion by appending 'CoordFrame'
    EXPECT_NO_THROW(getCoordinateFrameIdFromDeviceName("FrontLeftStereo"));
    EXPECT_NO_THROW(getCoordinateFrameIdFromDeviceName("FrontRightStereo"));
    EXPECT_NO_THROW(getCoordinateFrameIdFromDeviceName("RearLeftStereo"));
    EXPECT_NO_THROW(getCoordinateFrameIdFromDeviceName("RearRightStereo"));
    EXPECT_NO_THROW(getCoordinateFrameIdFromDeviceName("FrontFisheye"));
    EXPECT_NO_THROW(getCoordinateFrameIdFromDeviceName("RearFisheye"));
    EXPECT_NO_THROW(getCoordinateFrameIdFromDeviceName("LeftFisheye"));
    EXPECT_NO_THROW(getCoordinateFrameIdFromDeviceName("RightFisheye"));
    EXPECT_NO_THROW(getCoordinateFrameIdFromDeviceName("BaseLink"));

    // Not a valid name
    EXPECT_THROW(getCoordinateFrameIdFromDeviceName("Doh"), std::runtime_error);
}