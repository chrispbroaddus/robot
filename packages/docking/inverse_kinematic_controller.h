#pragma once

#include <atomic>

#include "Eigen/Eigen"
#include "glog/logging.h"
#include "inverse_kinematic_controller_helper.h"
#include "packages/core/proto/geometry.pb.h"
#include "packages/docking/proto/vehicle_calibration.pb.h"
#include "packages/hal/proto/vcu_ik_control_command.pb.h"
#include "packages/math/geometry/quaternion.h"
#include "thirdparty/Sophus/sophus/se3.hpp"
#include "vcu_ik_telemetry_state.h"

namespace docking {

///
/// Docking-planner based on inverse-kinematics
///
class InverseKinematicsController {

public:
    ///
    /// The perturbation on random variables.
    /// The random variables are chassis position (x & z), left/right rail height, and bearing angle (pitch)
    ///
    struct StatePerturbation {

        /// supplementary variable consistent with other perturbations
        float delta;

        Sophus::SE3<float> se3Perturb;
        float leftRailPerturb;
        float rightRailPerturb;
        float bearingAnglePerturb;
    };

    struct RawVCUIKControlCommand {
        float delta_wheel_center_x_in_meter;
        float delta_wheel_center_z_in_meter;
        float delta_wheel_center_rot_in_radian;
        float delta_left_slider_value;
        float delta_right_slider_value;
        float delta_left_rail_servo;
        float delta_right_rail_servo;
    };

    ///
    /// \param zippyCalibration
    ///
    InverseKinematicsController(const docking::VehicleCalibration& zippyCalibration);

    ///
    /// \param target
    /// \param command
    /// \return
    ///
    bool planTo(RawVCUIKControlCommand& command, const Sophus::SE3f& target, const VcuIkTelemetryState& telemetryState);

    //
    // Docking status
    //
    // -----------O------------> (+x)
    //            |
    //            V (+y)
    //
    //               ^  ^
    //        ---    |t/
    //       /   --- |/
    //      /       -Q-
    //     /           ---
    //                    ---
    //                      /
    // O : anchor point on the target station
    // Q : anchor point on the vehicle
    //
    // remaining_distance_x&y : Position of Q in O coordinate, in meters. (i.e., in this example : positive x & y value)
    // remaining_angle : The angle t in the figure, in radian.

    ///
    /// @brief returns the remaining_distance_y in meters, as the figure above
    ///
    float remainingDistancePerpendicularFromTarget() const { return -m_se3TwrtF.translation().coeff(0); }

    ///
    /// @brief returns the remaining_distance_x in meters, as in the figure above
    ///
    float remainingDistanceParallelFromTarget() const { return m_se3TwrtF.translation().coeff(1); }

    ///
    /// @brief returns the remaining angle `t` in radian, as in the figure above
    ///
    float remainingAngle() const {
        auto r = geometry::convertQuaternionToRodrigues(m_se3TwrtF.unit_quaternion().w(), m_se3TwrtF.unit_quaternion().x(),
            m_se3TwrtF.unit_quaternion().y(), m_se3TwrtF.unit_quaternion().z());
        return r(2, 0);
    }

private:
    ///
    /// \param statePerturbation
    /// \param variableIndex
    /// \return
    bool perturbStatePerVariable(StatePerturbation& statePerturbation, const VehiclePerturbedParameter parameter);

    ///
    docking::VehicleCalibration m_calib;

    Sophus::SE3<float> m_se3TwrtF;
};

} // docking
