#include <stdio.h>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "packages/planning/utils.h"

template <typename T> bool approximatelyEqual(const Sophus::SE3<T>& a, const Sophus::SE3<T>& b) {
    constexpr T kEpsilonPositionTolerance = 1.0e-5;
    constexpr T kEpsilonRotationTolerance = 1.0e-6;
    auto positionEps = (a.translation() - b.translation()).norm();
    auto rotationEps = 1 - pow(a.unit_quaternion().coeffs().transpose() * b.unit_quaternion().coeffs(), 2);
    if (positionEps > kEpsilonPositionTolerance) {
        LOG(ERROR) << positionEps;
    }
    if (rotationEps > kEpsilonRotationTolerance) {
        LOG(ERROR) << rotationEps;
    }
    return (positionEps < kEpsilonPositionTolerance && rotationEps < kEpsilonRotationTolerance);
}

TEST(utils, timeToDuration) {
    using core::Duration;
    using planning::toDuration;
    double rtime = 4.2;
    Duration time = toDuration(rtime);
    CHECK(time.nanos() == 4200000000);
}

TEST(utils, wykobiTransformConversion) {
    using planning::wykobiToZippy;
    using planning::zippyToWykobi;

    for (double x = -10; x < 10; x += 0.1) {
        for (double y = -10; y < 10; y += 0.1) {
            wykobi::point2d<double> point;
            point.x = x;
            point.y = y;
            core::Point2d intermediate;
            wykobiToZippy(point, intermediate);
            wykobi::point2d<double> result;
            zippyToWykobi(intermediate, result);
            ASSERT_EQ(point, result);
        }
    }
}

TEST(utils, sbplTransformTest) {
    using planning::elementsToPose;
    using planning::zippyToSBPL;
    using planning::SBPLToZippy;
    using Sophus::SE3d;

    for (double x = -10; x < 10; x += 1) {
        for (double y = -10; y < 10; y += 1) {
            for (double theta = -M_PI; y < M_PI; y += 1) {
                Sophus::SE3d pose;
                elementsToPose(x, y, 0.0, 0.0, 0.0, theta, pose);
                Sophus::SE3d intermediate, result;
                zippyToSBPL(pose, intermediate);
                SBPLToZippy(intermediate, result);
                ASSERT_TRUE(approximatelyEqual(pose, result));
            }
        }
    }
}

TEST(utils, sbplTransformTestEuler) {
    using planning::zippyToSBPL;
    using planning::elementsToPose;

    double input_angles[] = { 0, M_PI / 6, M_PI / 4, M_PI / 2, M_PI };
    double output_angles[] = { M_PI / 2, M_PI / 2 - M_PI / 6, M_PI / 2 - M_PI / 4, M_PI / 2 - M_PI / 2, M_PI / 2 - M_PI };

    int counter = 0;
    for (auto angle : input_angles) {
        Sophus::SE3d zippy;
        elementsToPose(0.0, 0.0, 0.0, 0.0, 0.0, angle, zippy);
        auto zippy_rpq = zippy.unit_quaternion().toRotationMatrix().eulerAngles(0, 1, 2);
        ASSERT_FLOAT_EQ(angle, zippy_rpq[planning::kZAxis]);

        Sophus::SE3d sbpl;
        zippyToSBPL(zippy, sbpl);

        auto sbpl_rpq = sbpl.unit_quaternion().toRotationMatrix().eulerAngles(0, 1, 2);
        ASSERT_NEAR(output_angles[counter], sbpl_rpq[planning::kZAxis], 1e-12);
        counter++;
    }
}

TEST(utils, transformToFromProtoConversion) {
    using planning::poseToProto;
    using planning::protoToPose;

    Sophus::SE3d pose_sophus(Eigen::Quaterniond{ 1, 0, 0, 0 }, Eigen::Vector3d{ 0, 0, 0 });

    calibration::CoordinateTransformation pose_zippy;
    poseToProto(pose_sophus, pose_zippy);
    Sophus::SE3d result;
    protoToPose(pose_zippy, result);
    ASSERT_TRUE(approximatelyEqual(pose_sophus, result));

    for (auto x = -M_PI; x <= M_PI; x += .01) {
        for (auto y = -M_PI; x <= M_PI; x += .01) {
            for (auto z = -M_PI; x <= M_PI; x += .01) {

                auto rotation = Eigen::AngleAxisd(x, Eigen::Vector3d::UnitX()) * Eigen::AngleAxisd(y, Eigen::Vector3d::UnitY())
                    * Eigen::AngleAxisd(z, Eigen::Vector3d::UnitZ());

                Sophus::SE3d sophus_input(rotation, Eigen::Vector3d{ x, y, z });
                poseToProto(sophus_input, pose_zippy);
                protoToPose(pose_zippy, result);
                ASSERT_TRUE(approximatelyEqual(sophus_input, result));
            }
        }
    }
}

TEST(utils, trajectorySampling) {
    using planning::Trajectory;
    using planning::sample;
    using core::Point2d;

    constexpr float kPathIncrement = 0.1;
    constexpr float kPathLength = 1.0;

    Trajectory trajectory;
    auto element = trajectory.add_elements();
    std::vector<core::Point2d> points;

    constexpr float kPositiveVelocity = 1.0;
    constexpr float kNegativeVelocity = -1.0;
    constexpr float kPositiveCurvature = 1.0;
    constexpr float kNegativeCurvature = -1.0;
    constexpr int kNumPoints = kPathLength / kPathIncrement;

    element->set_arclength(kPathLength);

    // Front-right quadrant
    element->set_curvature(kPositiveCurvature);
    element->set_linear_velocity(kPositiveVelocity);
    ASSERT_TRUE(sample(trajectory.elements(0), points, kPathIncrement));
    ASSERT_EQ(points.size(), kNumPoints);
    for (auto point : points) {
        ASSERT_GE(point.x(), 0);
        ASSERT_GE(point.y(), 0);
    }

    // Rear-right quadrant
    element->set_curvature(kPositiveCurvature);
    element->set_linear_velocity(kNegativeVelocity);
    ASSERT_TRUE(sample(trajectory.elements(0), points, kPathIncrement));
    ASSERT_EQ(points.size(), kNumPoints);
    for (auto point : points) {
        CHECK_GE(point.y(), 0);
        CHECK_LE(point.x(), 0);
    }

    // Front-left quadrant
    element->set_curvature(kNegativeCurvature);
    element->set_linear_velocity(kPositiveVelocity);
    ASSERT_TRUE(sample(trajectory.elements(0), points, kPathIncrement));
    ASSERT_EQ(points.size(), kNumPoints);
    for (auto point : points) {
        CHECK_GE(point.x(), 0);
        CHECK_LE(point.y(), 0);
    }

    // Rear-left quadrant
    element->set_curvature(kNegativeCurvature);
    element->set_linear_velocity(kNegativeVelocity);
    ASSERT_TRUE(sample(trajectory.elements(0), points, kPathIncrement));
    ASSERT_EQ(points.size(), kNumPoints);
    for (auto point : points) {
        CHECK_LE(point.x(), 0);
        CHECK_LE(point.y(), 0);
    }
}

TEST(utils, fileCreator) {
    using planning::FilenameCreator;
    constexpr int kFileDoesNotExist = -1;
    constexpr int kNumFilesToCreate = 10;
    std::set<std::string> filenames;
    {
        FilenameCreator creator("utils_file_creator_test");
        for (int i = 0; i < kNumFilesToCreate; ++i) {
            auto filename = creator();
            ASSERT_NE(access(filename.c_str(), F_OK), kFileDoesNotExist);
            filenames.insert(filename);
        }
    }
    ASSERT_EQ(filenames.size(), kNumFilesToCreate);
    for (const auto& filename : filenames) {
        ASSERT_EQ(access(filename.c_str(), F_OK), kFileDoesNotExist);
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
