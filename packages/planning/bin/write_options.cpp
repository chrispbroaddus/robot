#include <fstream>

#include "glog/logging.h"
#include "google/protobuf/text_format.h"

#include "packages/planning/planner.h"
#include "packages/planning/proto/trajectory_options.pb.h"
#include "packages/planning/proto/trajectory_planner_options.pb.h"

int main() {
    using planning::TrajectoryPlannerOptions;
    TrajectoryPlannerOptions base_options;
    base_options.set_frequency(10);
    {
        std::string text;
        google::protobuf::TextFormat::PrintToString(base_options, &text);
        std::fstream output("planner_options.default.pbtxt", std::ios::out);
        output << text;
        output.close();
    }

    using planning::ArcPlannerOptions;
    using planning::TrajectoryType;
    ArcPlannerOptions arc_options;
    arc_options.mutable_base_options()->MergeFrom(base_options);
    arc_options.set_minimum_x_tolerance(0.01);
    arc_options.set_max_curvature(1.0 / 15.0);
    arc_options.set_type(TrajectoryType::SQUARE);
    arc_options.set_max_curvature(1.0);
    {
        std::string text;
        google::protobuf::TextFormat::PrintToString(arc_options, &text);
        std::fstream output("arc_planner_options.default.pbtxt", std::ios::out);
        output << text;
        output.close();
    }

    using planning::TrajectoryOptions;
    TrajectoryOptions trajectory_options;
    trajectory_options.set_max_velocity(1.0);
    trajectory_options.set_max_acceleration(5.0);
    {
        std::string text;
        google::protobuf::TextFormat::PrintToString(trajectory_options, &text);
        std::fstream output("trajectory_options.default.pbtxt", std::ios::out);
        output << text;
        output.close();
    }
}
