#include "glog/logging.h"
#include "packages/executor/logging.h"
#include "packages/planning/logging.h"
#include "gtest/gtest.h"

TEST(utils, logging) {
    using core::Point3d;
    using executor::Logger;
    using executor::LogEntry;
    using planning::readLog;
    using planning::FilenameCreator;

    FilenameCreator creator("utils_logging_test");
    auto filename = creator();

    constexpr int kNumEntries = 10000;
    {
        Logger logger(filename);
        for (int i = 0; i < kNumEntries; ++i) {
            Point3d point;
            point.set_x(i);
            point.set_x(i);
            point.set_x(i);
            logger.log(point);
        }
    }
    std::deque<LogEntry> point_and_go_entries;
    ASSERT_TRUE(readLog(filename, point_and_go_entries));
    ASSERT_EQ(kNumEntries, point_and_go_entries.size());

    using planning::Trajectory;
    {
        Logger logger(filename);
        for (int i = 0; i < kNumEntries; ++i) {
            Trajectory trajectory;
            for (int j = 0; j < 100; ++j) {
                auto element = trajectory.add_elements();
                element->set_curvature(i);
                element->set_linear_velocity(j);
            }
            logger.log(trajectory);
        }
    }
    std::deque<LogEntry> trajectory_entries;
    ASSERT_TRUE(readLog(filename, trajectory_entries));
    ASSERT_EQ(kNumEntries, trajectory_entries.size());

    {
        Logger logger(filename);
        for (int i = 0; i < kNumEntries; ++i) {
            if (i % 2 == 0) {
                Trajectory trajectory;
                for (int j = 0; j < 100; ++j) {
                    auto element = trajectory.add_elements();
                    element->set_curvature(i);
                    element->set_linear_velocity(j);
                }
                logger.log(trajectory);
            } else {
                Point3d point;
                point.set_x(i);
                point.set_x(i);
                point.set_x(i);
                logger.log(point);
            }
        }
    }
    std::deque<LogEntry> interleaved_entries;
    ASSERT_TRUE(readLog(filename, interleaved_entries));
    ASSERT_EQ(kNumEntries, interleaved_entries.size());
    for (size_t i = 0; i < interleaved_entries.size(); ++i) {
        auto entry = interleaved_entries[i];
        if (i % 2 == 0) {
            ASSERT_TRUE(entry.has_planned_trajectory());
            ASSERT_FALSE(entry.has_point_and_go_request());
        } else {
            ASSERT_TRUE(entry.has_point_and_go_request());
            ASSERT_FALSE(entry.has_planned_trajectory());
        }
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
