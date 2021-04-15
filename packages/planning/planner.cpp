#include "packages/planning/planner.h"

#include <unistd.h>

#include "boost/msm/back/state_machine.hpp"
#include "boost/msm/front/euml/common.hpp"
#include "boost/msm/front/functor_row.hpp"
#include "boost/msm/front/state_machine_def.hpp"
#include "glog/logging.h"
#include "sbpl/headers.h"
#include "sbpl/utils/utils.h"
#include "wykobi/wykobi.hpp"

#include "packages/planning/utils.h"

namespace planning {

static const wykobi::point2d<double> kOrigin = wykobi::make_point(0.0, 0.0);
static const wykobi::point2d<double> kUnitX = wykobi::make_point(1.0, 0.0);
constexpr double kMinimumLateralTolerance = 0.001;
constexpr double kMaxPlannerHorizon = 10.0;
constexpr char kDefaultTrajectoryPlannerOptions[] = "config/global/trajectory_planner_options.default.pbtxt";
constexpr char kDefaultArcPlannerOptions[] = "config/global/arc_planner_options.default.pbtxt";

TrajectoryPlannerOptions loadDefaultTrajectoryPlannerOptions() {
    TrajectoryPlannerOptions options;
    CHECK(loadOptions(kDefaultTrajectoryPlannerOptions, options));
    return options;
}

ArcPlannerOptions loadDefaultArcPlannerOptions() {
    ArcPlannerOptions options;
    CHECK(loadOptions(kDefaultArcPlannerOptions, options));
    return options;
}

/*
 *Compute the intersection point of an arc that passes through (0,0) with the
 * target point, with the centre constrained to live on the (zippy) y axis.
 * This function works in the wykobi (i.e. Matlab) frame, and so helper
 * functions are required to convert between the zippy frame and wykobi.
 */
bool computeIntersectionPoint(const wykobi::point3d<double>& target, wykobi::point2d<double>& point) {
    auto arc_end = wykobi::make_point(target.x, target.y);
    wykobi::circle<double> circumscribe = wykobi::make_circle(kOrigin, arc_end);

    auto line1 = wykobi::tangent_line(circumscribe, arc_end);
    auto line2 = wykobi::make_line(kOrigin, kUnitX);
    if (!wykobi::intersect(line1, line2)) {
        return false;
    }
    point = wykobi::intersection_point(line1, line2);

    if (target.x < 0) {
        CHECK(point.x <= 0);
    } else {
        CHECK(point.x >= 0);
    }
    CHECK(point.y == 0);
    return true;
}

double computeArcLength(const wykobi::point3d<double>& target, const wykobi::point2d<double>& intersection) {
    CHECK(intersection.y == 0.0);
    wykobi::point2d<double> diff;
    diff.x = std::fabs(target.x - intersection.x);
    diff.y = std::fabs(target.y - intersection.y);
    const double a = wykobi::vector_norm(wykobi::make_vector<double>(intersection));
    const double b = std::atan2(diff.y, diff.x);
    return std::abs(a * b);
}

// Define a skeletal State Machine for the planner. The machine has 4 states:
// 1. Uninitialized
//      Planner has yet to be initialized
// 2. Active
//      Plan actively being generated. For simple arc-based planners this is
//      trivial, but more complex planners will take longer to plan.
// 3. Inactive
//      Planner is ready to receive a planning request.
// 4. Error
//      The last requested plan resulted in an error.
class PlannerStateMachine_ : public boost::msm::front::state_machine_def<PlannerStateMachine_> {
public:
    struct Uninitialized : public boost::msm::front::state<> {
        template <class Event, class FSM> void on_entry(__attribute__((unused)) Event const& e, FSM) { LOG(INFO) << "UNINITIALIZED"; }
    };

    typedef Uninitialized initial_state;

    struct Active : public boost::msm::front::state<> {
        template <class Event, class FSM> void on_entry(__attribute__((unused)) Event const& e, FSM) { LOG(INFO) << "ACTIVE"; }
    };

    struct Inactive : public boost::msm::front::state<> {
        template <class Event, class FSM> void on_entry(__attribute__((unused)) Event const& e, FSM) { LOG(INFO) << "INACTIVE"; }
    };

    struct Error : public boost::msm::front::state<> {
        template <class Event, class FSM> void on_entry(__attribute__((unused)) Event const& e, FSM) { LOG(INFO) << "ERROR"; }
    };

    struct InternalPlannerState {
        InternalPlannerState(PlannerState& state)
            : m_state(state) {}
        PlannerState& m_state;
    };

    // Planner initialization
    struct initialize : InternalPlannerState {
        initialize(PlannerState& state)
            : InternalPlannerState(state) {}
    };
    void do_initialization(initialize const& event) { event.m_state = PlannerState::PLANNER_INACTIVE; }

    // Planning methods
    struct plan : InternalPlannerState {
        plan(PlannerState& state)
            : InternalPlannerState(state) {}
    };
    void do_plan(plan const& event) { event.m_state = PlannerState::PLANNER_ACTIVE; }
    void do_interrupt(__attribute__((unused)) const plan& event) { LOG(ERROR) << "Planner interrupt currently unsupported!"; }

    // Planning results
    struct success : InternalPlannerState {
        success(PlannerState& state)
            : InternalPlannerState(state) {}
    };
    void do_success(const success& event) { event.m_state = PlannerState::PLANNER_INACTIVE; }
    struct failure : InternalPlannerState {
        failure(PlannerState& state)
            : InternalPlannerState(state) {}
    };
    void do_failure(const failure& event) { event.m_state = PlannerState::PLANNER_FAILED; }

    struct timeout : InternalPlannerState {
        timeout(PlannerState& state)
            : InternalPlannerState(state) {}
    };
    void do_timeout(const timeout&) {}

    struct transition_table
        : boost::mpl::vector<
              //    Start                  Event               Next              Action               Guard
              //  +--------------+---------------------+----------------+---------------------+----------------------+
              a_row<Uninitialized, initialize, Inactive, &PlannerStateMachine_::do_initialization>,
              a_row<Inactive, plan, Active, &PlannerStateMachine_::do_plan>, a_row<Error, plan, Active, &PlannerStateMachine_::do_plan>,
              a_row<Active, success, Inactive, &PlannerStateMachine_::do_success>,
              a_row<Active, failure, Error, &PlannerStateMachine_::do_failure>,
              a_row<Active, timeout, Error, &PlannerStateMachine_::do_timeout>,
              a_row<Active, plan, Active, &PlannerStateMachine_::do_interrupt> > {};

    template <class FSM, class Event> void no_transition(Event const& event, FSM&, int) {
        std::stringstream error;
        error << "no_transition called!";
        LOG(ERROR) << error.str();
        event.m_state = PlannerState::PLANNER_FAILED;
    }
};

struct StateMachineAdapter::StateMachineAdapterImpl {
    typedef boost::msm::back::state_machine<PlannerStateMachine_> PlannerStateMachine;

    StateMachineAdapterImpl()
        : m_stateMachine(new PlannerStateMachine) {}

    std::unique_ptr<PlannerStateMachine> m_stateMachine;
};

StateMachineAdapter::StateMachineAdapter(std::shared_ptr<TrajectoryPlanner> planner)
    : TrajectoryPlanner(planner->options())
    , m_planner(planner)
    , m_impl(new StateMachineAdapterImpl) {
    m_impl->m_stateMachine->start();
    m_impl->m_stateMachine->process_event(PlannerStateMachine_::initialize(m_state));
}

bool StateMachineAdapter::planTo(const Sophus::SE3d& target, Trajectory& trajectory) {
    m_impl->m_stateMachine->process_event(PlannerStateMachine_::plan(m_state));
    bool success = m_planner->planTo(target, trajectory);
    success ? m_impl->m_stateMachine->process_event(PlannerStateMachine_::success(m_state))
            : m_impl->m_stateMachine->process_event(PlannerStateMachine_::failure(m_state));
    return success;
}

bool ArcPlanner::planTo(const Sophus::SE3d& target, Trajectory& trajectory) {
    if (target.translation()[kXAxis] > kMaxPlannerHorizon) {
        LOG(ERROR) << "Goal exceeds planner horizon";
        return false;
    }
    int direction = target.translation()[kXAxis] >= 0 ? 1 : -1;

    wykobi::point3d<double> _target;
    zippyToWykobi(target, _target);

    if (std::abs(_target[kXAxis]) < kMinimumLateralTolerance) {
        _target[kXAxis] = kMinimumLateralTolerance;
    }

    // Compute arc intersection point
    wykobi::point2d<double> _intersection_point;
    if (!computeIntersectionPoint(_target, _intersection_point)) {
        return false;
    }

    // Compute arc length
    auto arc_length = computeArcLength(_target, _intersection_point);
    CHECK(arc_length > 0);

    // Compute curvature
    core::Point2d intersection_point;
    wykobiToZippy(_intersection_point, intersection_point);
    CHECK(intersection_point.x() == 0);
    auto curvature = 1.0 / (intersection_point.y() / 2.0);

    CHECK(m_generator->generate(arc_length, curvature, trajectory));
    CHECK(trajectory.elements_size() > 0);

    for (auto& element : *trajectory.mutable_elements()) {
        element.set_linear_velocity(element.linear_velocity() * direction);
    }
    // Increment trajectory counter
    ++m_counter;
    return true;
}

class SBPLPlanner::SBPLPlannerImpl {
    static constexpr double kDefaultGridWidth = 20.0; // (m)
    static constexpr double kDefaultGridHeight = 20.0; // (m)
    static constexpr double kDefaultPlannerTimeHorizon = 5.0; // (s)
    static constexpr double kGoalToleranceX = 0.1; // (m)
    static constexpr double kGoalToleranceY = 0.1; // (m)
    static constexpr double kGoalToleranceTheta = M_PI / 20.0; // (rad)
    static constexpr double kNominalForwardVelocity = 1.0; // (m/s)
    static constexpr double kTimeToRotateInPlace = 5.0; // (s)
    static constexpr double kObstacleThreshold = 1.0; // (dimensionless)

public:
    SBPLPlannerImpl(const SBPLPlannerOptions& options)
        : m_options(options)
        , kDefaultGridResolution(0.05) {
        m_environment.reset(new EnvironmentNAVXYTHETALAT);
        m_width = kDefaultGridWidth / kDefaultGridResolution;
        m_height = kDefaultGridHeight / kDefaultGridResolution;
        const char motion_primitives[] = "thirdparty/sbpl/matlab/mprim/z2.mprim";
        CHECK(access(motion_primitives, F_OK) != -1) << "Could not access: " << motion_primitives;
        CHECK(m_environment->InitializeEnv(m_width, m_height,
            nullptr, // no costmap, currently
            0, 0, 0, // start
            0, 0, 0, // goal
            kGoalToleranceX, kGoalToleranceY, kGoalToleranceTheta, // tolerances (m, m, rad)
            {}, // robot perimeter (list, (m))
            kDefaultGridResolution, // grid size (m). This MUST match what is defined in `motion_primitives`
            kNominalForwardVelocity, // nominal forward v (m/s)
            kTimeToRotateInPlace, // time to rotate in place (s)
            kObstacleThreshold, // obstacle threshold (what value in the costmap tells us this is an obstacle) (unitless)
            motion_primitives));
        switch (options.planner_type()) {
        case SBPLPlannerOptions::ARASTAR:
            m_planner.reset(new ARAPlanner(m_environment.get(), false));
            break;
        default:
            throw std::runtime_error("Not implemented!");
        }
    }

    bool planTo(const Sophus::SE3d& zippy_target, Path& path) {
        double offset_x = (m_width * kDefaultGridResolution) / 2.0;
        CHECK(offset_x > 0);
        double offset_y = (m_height * kDefaultGridResolution) / 2.0;
        CHECK(offset_y > 0);

        // Add in a translation, so we are in the grid centre
        Eigen::Vector3d offset{ offset_x, offset_y, 0.0 };

        // Set starting position (SBPL frame)
        Sophus::SE3d zippy_start;
        identity(zippy_start);

        elementsToPose(offset_x, offset_y, 0.0, 0.0, 0.0, 0.0, zippy_start);
        Sophus::SE3d start;
        identity(start);

        zippyToSBPL(zippy_start, start);
        double x, y, z, r, p, q;
        poseToElements(x, y, z, r, p, q, start);
        CHECK(m_environment->SetStart(x, y, q) >= 0);

        // Set target(SBPL frame)
        Sophus::SE3d target;
        identity(target);

        zippyToSBPL(zippy_target, target);
        poseToElements(x, y, z, r, p, q, target);
        CHECK(m_environment->SetGoal(x + offset_x, y + offset_y, q) >= 0);
        MDPConfig config;
        CHECK(m_environment->InitializeMDPCfg(&config));

        auto timer_start = std::chrono::high_resolution_clock::now();
        CHECK(m_planner->force_planning_from_scratch_and_free_memory() != 0);
        CHECK(m_planner->set_start(config.startstateid) != 0);
        CHECK(m_planner->set_goal(config.goalstateid) != 0);

        std::vector<int> ids;
        CHECK(m_planner->replan(kDefaultPlannerTimeHorizon, &ids));
        LOG(INFO) << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - timer_start).count()
                  << " elapsed";
        if (ids.empty()) {
            return false;
        }
        std::vector<sbpl_xy_theta_pt_t> _path;
        m_environment->ConvertStateIDPathintoXYThetaPath(&ids, &_path);
        for (const auto& point : _path) {
            auto element = path.add_elements();
            Sophus::SE3d sbpl_element;
            identity(sbpl_element);
            elementsToPose(point.x - offset_x, point.y - offset_y, 0.0, 0.0, 0.0, point.theta, sbpl_element);
            Sophus::SE3d zippy_element;
            identity(zippy_element);
            SBPLToZippy(sbpl_element, zippy_element);
            calibration::CoordinateTransformation transform;
            poseToProto(zippy_element, transform);
            *element->mutable_transform() = transform;
        }
        if (path.elements_size() > 0) {
            auto initialLateralOffset = path.elements(0).transform().translationy();
            auto initialLongitudinalOffset = path.elements(0).transform().translationx();
            // We assume that the planner discretization offset is non-zero
            CHECK(std::abs(initialLateralOffset) > 0);
            for (auto& element : *path.mutable_elements()) {
                auto lateralOffset = element.transform().translationy();
                element.mutable_transform()->set_translationy(lateralOffset - initialLateralOffset);
                auto longitudinalOffset = element.transform().translationx();
                element.mutable_transform()->set_translationx(longitudinalOffset - initialLongitudinalOffset);
            }
        }
        return (!ids.empty());
    }

private:
    const SBPLPlannerOptions& m_options;
    const double kDefaultGridResolution;
    std::unique_ptr<ARAPlanner> m_planner;
    std::unique_ptr<EnvironmentNAVXYTHETALAT> m_environment;
    int m_width, m_height;
};

SBPLPlanner::SBPLPlanner(const SBPLPlannerOptions& options)
    : PathPlanner()
    , m_impl(new SBPLPlannerImpl(options)) {}

bool SBPLPlanner::planTo(const Sophus::SE3d& target, Path& path) { return m_impl->planTo(target, path); }

} // planning
