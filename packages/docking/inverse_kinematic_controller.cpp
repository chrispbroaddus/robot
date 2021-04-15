#include "inverse_kinematic_controller.h"
#include "inverse_kinematic_controller_helper.h"
#include "packages/math/geometry/quaternion.h"

namespace docking {

InverseKinematicsController::InverseKinematicsController(const VehicleCalibration& zippyCalibration)
    : m_calib{ zippyCalibration } {
    VLOG(1) << __PRETTY_FUNCTION__ << " constructed.";
    VLOG(1) << " zippyCalibration.distBetweenWheel : " << zippyCalibration.distbetweenwheel();
    VLOG(1) << " m_calib.distBetweenWheel : " << m_calib.distbetweenwheel();

    Eigen::Quaternionf q(1, 0, 0, 0);
    m_se3TwrtF.setQuaternion(q);
    m_se3TwrtF.translation() = Eigen::Vector3f(1000, 1000, 1000);
}

bool InverseKinematicsController::planTo(
    RawVCUIKControlCommand& command, const Sophus::SE3f& se3CwrtT, const VcuIkTelemetryState& telemetryState) {

    Sophus::SE3f se3JpwrtF;
    auto qJpwrtF = geometry::convertRodriguesToQuaternion<float>(
        m_calib.se3jpwrtf().rodriguesrotationx(), m_calib.se3jpwrtf().rodriguesrotationy(), m_calib.se3jpwrtf().rodriguesrotationz());
    se3JpwrtF.setQuaternion(qJpwrtF);
    se3JpwrtF.translation()
        = Eigen::Vector3f(m_calib.se3jpwrtf().translationx(), m_calib.se3jpwrtf().translationy(), m_calib.se3jpwrtf().translationz());

    VLOG(2) << __FUNCTION__ << " ... se3JpwrtF : " << se3JpwrtF.matrix3x4();

    Sophus::SE3f se3FwrtC;
    auto qFwrtC = geometry::convertRodriguesToQuaternion<float>(
        m_calib.se3fwrtc().rodriguesrotationx(), m_calib.se3fwrtc().rodriguesrotationy(), m_calib.se3fwrtc().rodriguesrotationz());
    se3FwrtC.setQuaternion(qFwrtC);
    se3FwrtC.translation()
        = Eigen::Vector3f(m_calib.se3fwrtc().translationx(), m_calib.se3fwrtc().translationy(), m_calib.se3fwrtc().translationz());

    VLOG(2) << __FUNCTION__ << " ... se3FwrtC : " << se3FwrtC.matrix3x4();

    // Calculate the pose between the current chassis bottom wrt the vehicle's anchor point using the rigid transformation chain
    Sophus::SE3f se3OwrtF;
    getSE3ChassisCenterWrtVehicleDockAnchor(se3OwrtF, telemetryState, se3JpwrtF, (float)m_calib.distbetweenwheel());

    Sophus::SE3<float> se3OwrtT = se3OwrtF * se3FwrtC * se3CwrtT;

    // Current Pose
    VLOG(2) << __FUNCTION__ << " ... quatOwrtF : " << se3OwrtF.so3().matrix();
    VLOG(2) << __FUNCTION__ << "... transOwrtF : " << se3OwrtF.translation();

    // Calculate Numerical Jacobian
    Eigen::Matrix<float, 6, 9> J = Eigen::Matrix<float, 6, 9>::Zero();
    Eigen::Matrix<float, 6, 1> dx = Eigen::Matrix<float, 6, 1>::Zero();

    Sophus::SE3<float> se3FwrtT = se3OwrtF.inverse() * se3OwrtT;
    m_se3TwrtF = se3FwrtT.inverse();
    VLOG(1) << __FUNCTION__ << " ... remaining translation (target anchor wrt vehicle anchor) : " << m_se3TwrtF.translation();

    VLOG(2) << " ==== Calculating dx ... : ";
    dx.block(0, 0, 6, 1) = divideSE3OverDelta<float>(1.f, se3FwrtT);

    // Calculate column-wise numerical jacobian
    for (int c = 0; c < 9; c++) {
        VehiclePerturbedParameter parameter;
        switch (c) {
        case 0:
            // +X direction : vehicle forward
            parameter = VehiclePerturbedParameter::ChassisXPosition;
            break;
        case 1:
            // +Y direction : vehicle right
            parameter = VehiclePerturbedParameter::ChassisYPosition;
            break;
        case 3:
            parameter = VehiclePerturbedParameter::ChassisRotationRadians;
            break;
        case 6:
            parameter = VehiclePerturbedParameter::LeftRailHeight;
            break;
        case 7:
            parameter = VehiclePerturbedParameter::RightRailHeight;
            break;
        case 8:
            parameter = VehiclePerturbedParameter::RotatinPitch;
            break;
        default:
            continue;
        }

        StatePerturbation statePerturbation;
        statePerturbation.se3Perturb = Sophus::SE3<float>(Eigen::Quaternion<float>(1, 0, 0, 0), Eigen::Matrix<float, 3, 1>(0, 0, 0));
        statePerturbation.leftRailPerturb = 0;
        statePerturbation.rightRailPerturb = 0;
        statePerturbation.bearingAnglePerturb = 0;

        if (perturbStatePerVariable(statePerturbation, parameter)) {
            VLOG(2) << " ==== Calculating J[:," << c << "] ... : ";
            J.block(0, c, 6, 1) = getJacobianPerColumn(statePerturbation.delta, se3OwrtF, statePerturbation.se3Perturb,
                statePerturbation.leftRailPerturb, statePerturbation.rightRailPerturb, statePerturbation.bearingAnglePerturb,
                telemetryState, se3JpwrtF, static_cast<float>(m_calib.distbetweenwheel()));
        }
    }

    auto dp = J.transpose() * dx;

    // Gradient descent update
    command.delta_wheel_center_x_in_meter = dp(1, 0); // opposite to gradient since we are in opposite coordinate
    command.delta_wheel_center_z_in_meter = dp(0, 0); // opposite to gradient since we are in opposite coordinate
    command.delta_wheel_center_rot_in_radian = -dp(3, 0);
    command.delta_left_slider_value = -dp(6, 0);
    command.delta_right_slider_value = -dp(7, 0);
    command.delta_left_rail_servo = -dp(8, 0); // left and right servo should rotate together.
    command.delta_right_rail_servo = -dp(8, 0); // left and right servo should rotate together.

    return true;
}

bool InverseKinematicsController::perturbStatePerVariable(StatePerturbation& statePerturbation, const VehiclePerturbedParameter parameter) {
    return perturbState(statePerturbation.delta, statePerturbation.se3Perturb, statePerturbation.leftRailPerturb,
        statePerturbation.rightRailPerturb, statePerturbation.bearingAnglePerturb, parameter);
}

} // docking_manager
