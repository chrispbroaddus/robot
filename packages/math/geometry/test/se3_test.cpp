#include "packages/math/geometry/se3.h"
#include "Eigen/Eigen"
#include "glog/logging.h"
#include "gtest/gtest.h"

using namespace geometry;

TEST(Geometry, getSophusSE3Idempotence) {

    calibration::CoordinateTransformation t;
    t.set_translationx(1);
    t.set_translationy(3);
    t.set_translationz(2);
    t.set_rodriguesrotationx(-0.09999981);
    t.set_rodriguesrotationy(0);
    t.set_rodriguesrotationz(-0.499999);

    auto se3 = getSophusSE3<double>(t);
    Eigen::AngleAxisd angleAxisd(se3.unit_quaternion());

    EXPECT_NEAR(se3.translation().coeff(0), t.translationx(), 1e2 * std::numeric_limits<double>::epsilon());
    EXPECT_NEAR(se3.translation().coeff(1), t.translationy(), 1e2 * std::numeric_limits<double>::epsilon());
    EXPECT_NEAR(se3.translation().coeff(2), t.translationz(), 1e2 * std::numeric_limits<double>::epsilon());
    EXPECT_NEAR(angleAxisd.axis().coeff(0) * angleAxisd.angle(), t.rodriguesrotationx(), 1e2 * std::numeric_limits<double>::epsilon());
    EXPECT_NEAR(angleAxisd.axis().coeff(1) * angleAxisd.angle(), t.rodriguesrotationy(), 1e2 * std::numeric_limits<double>::epsilon());
    EXPECT_NEAR(angleAxisd.axis().coeff(2) * angleAxisd.angle(), t.rodriguesrotationz(), 1e2 * std::numeric_limits<double>::epsilon());
}
