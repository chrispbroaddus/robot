#pragma once

#include "Eigen/Eigen"
#include "glog/logging.h"
#include "glog/logging.h"
#include "google/protobuf/stubs/status.h"

#include "packages/planning/definitions.h"

namespace planning {
template <typename T> core::Duration toDuration(T inputInSeconds) {
    core::Duration time;
    time.set_nanos(std::floor(inputInSeconds * PhysicalConstants<T>::kSecondsToNanoseconds));
    return time;
}

template <typename T> void wykobiToZippy(const wykobi::point2d<T>& input, core::Point2d& output) {
    output.set_x(input.y);
    output.set_y(input.x);
}

template <typename T> void zippyToWykobi(const core::Point2d& input, wykobi::point2d<T>& output) {
    output.x = input.y();
    output.y = input.x();
}

template <typename T> void wykobiToZippy(const wykobi::point3d<T>& input, core::Point3d& output) {
    output.set_x(input.y);
    output.set_y(input.x);
    output.set_z(-1 * input.z);
}

template <typename T> void zippyToWykobi(const core::Point3d& input, wykobi::point3d<T>& output) {
    output.x = input.y();
    output.y = input.x();
    output.z = -1 * input.z();
}

template <typename T> void zippyToWykobi(const Sophus::SE3<T>& input, wykobi::point3d<T>& output) {
    const auto translation = input.translation();
    output.x = translation[1];
    output.y = translation[0];
    output.z = -1 * translation[2];
}

template <typename T> void zippyToWykobi(const Sophus::SE3<T>& input, wykobi::point2d<T>& output) {
    const auto translation = input.translation();
    output.x = translation()[1];
    output.y = translation()[0];
}

template <typename T> void poseToProto(const Sophus::SE3<T>& in, calibration::CoordinateTransformation& out) {
    Eigen::AngleAxis<T> aa(in.unit_quaternion());
    Eigen::Matrix<T, 1, 3> scaled;
    scaled[0] = aa.axis()[0] * aa.angle();
    scaled[1] = aa.axis()[1] * aa.angle();
    scaled[2] = aa.axis()[2] * aa.angle();

    out.set_rodriguesrotationx(scaled[0]);
    out.set_rodriguesrotationy(scaled[1]);
    out.set_rodriguesrotationz(scaled[2]);

    const auto& translation = in.translation();
    out.set_translationx(translation[0]);
    out.set_translationy(translation[1]);
    out.set_translationz(translation[2]);
}

template <typename T> void protoToPose(const calibration::CoordinateTransformation& in, Sophus::SE3<T>& out) {
    Eigen::Matrix<T, 1, 3> axis(in.rodriguesrotationx(), in.rodriguesrotationy(), in.rodriguesrotationz());
    auto angle = axis.norm();

    Eigen::Quaternion<T> rotation;

    if (angle == 0.0) {
        rotation = Eigen::Quaternion<T>(Eigen::AngleAxis<T>(0, Eigen::Matrix<T, 1, 3>::Zero()));
    } else {
        axis /= angle;
        rotation = Eigen::Quaternion<T>(Eigen::AngleAxis<T>(angle, axis));
    }
    out = Sophus::SE3<T>(rotation, Eigen::Matrix<T, 1, 3>{ in.translationx(), in.translationy(), in.translationz() });
}

template <typename T> void _convert(const Sophus::SE3<T>& input, Sophus::SE3<T>& output) {
    auto angles = input.unit_quaternion().toRotationMatrix().eulerAngles(0, 1, 2);
    double yaw = M_PI / 2.0 - angles[kZAxis];
    auto translation = input.translation();
    translation.row(kXAxis).swap(translation.row(kYAxis));
    translation.row(kZAxis) *= -1;
    output = Sophus::SE3<T>(Eigen::AngleAxis<T>(0, Eigen::Matrix<T, 1, 3>::UnitX())
            * Eigen::AngleAxis<T>(0, Eigen::Matrix<T, 1, 3>::UnitY()) * Eigen::AngleAxis<T>(yaw, Eigen::Matrix<T, 1, 3>::UnitZ()),
        translation);
}

template <typename T> void zippyToSBPL(const Sophus::SE3<T>& input, Sophus::SE3<T>& output) { _convert(input, output); }

template <typename T> void SBPLToZippy(const Sophus::SE3<T>& input, Sophus::SE3<T>& output) { _convert(input, output); }

template <typename T> void zippyToOpenGL(const Sophus::SE3<T>& input, Sophus::SE3<T>& output) {
    Sophus::SE3d tmp(Eigen::Quaterniond{ 1, 0, 0, 0 }, Eigen::Vector3d{ 0, 0, 0 });
    elementsToPose(0.0, 0.0, 0.0, M_PI, 0.0, -M_PI / 2.0, tmp);
    output = tmp * input;
}

template <typename T> void zippyToOpenGL(const Eigen::Matrix<T, 4, 1>& input, Eigen::Matrix<T, 4, 1>& output) {
    Sophus::SE3d input_(Eigen::Quaterniond{ 1, 0, 0, 0 }, Eigen::Vector3d{ 0, 0, 0 });
    Sophus::SE3d output_ = input_;
    zippyToOpenGL(input_, output_);
    output = output_.matrix() * input;
}

template <typename T> void zippyToOpenGL(const Eigen::Matrix<T, 3, 1>& input, Eigen::Matrix<T, 3, 1>& output) {
    Sophus::SE3d input_(Eigen::Quaterniond{ 1, 0, 0, 0 }, Eigen::Vector3d{ 0, 0, 0 });
    Sophus::SE3d output_ = input_;
    zippyToOpenGL(input_, output_);
    output = output_ * input;
}

template <typename T> void pointToPose(const core::Point3d& point, Sophus::SE3<T>& pose) {
    pose = Sophus::SE3<T>(Eigen::Quaternion<T>{ 1, 0, 0, 0 }, Eigen::Matrix<T, 1, 3>{ point.x(), point.y(), point.z() });
}

template <typename T> void elementsToPose(T x, T y, T z, T r, T p, T q, Sophus::SE3<T>& pose) {
    auto rotation = Eigen::AngleAxisd(r, Eigen::Vector3d::UnitX()) * Eigen::AngleAxisd(p, Eigen::Vector3d::UnitY())
        * Eigen::AngleAxisd(q, Eigen::Vector3d::UnitZ());
    pose = Sophus::SE3<T>(rotation, Eigen::Vector3d{ x, y, z });
}

template <typename T> void poseToElements(T& x, T& y, T& z, T& r, T& p, T& q, const Sophus::SE3<T>& pose) {
    const auto angles = pose.unit_quaternion().toRotationMatrix().eulerAngles(0, 1, 2);
    r = angles[kXAxis];
    p = angles[kYAxis];
    q = angles[kZAxis];
    const auto translation = pose.translation();
    x = translation[kXAxis];
    y = translation[kYAxis];
    z = translation[kZAxis];
}

template <typename T> void transform(const Sophus::SE3<T> transform, Path& path) {
    for (auto& element : *path.mutable_elements()) {
        Sophus::SE3<T> se3_element;
        protoToPose(element.transform(), se3_element);
        poseToProto(transform * se3_element, *element.mutable_transform());
    }
}

template <class T> bool checkForNumeric(const Sophus::SE3<T>& element) { return element.matrix().allFinite(); }

template <class T> void identity(Sophus::SE3<T>& element) {
    const auto translation = Eigen::Matrix<T, 3, 1>{ 0, 0, 0 };
    const auto rotation = Eigen::Quaternion<T>{ 1, 0, 0, 0 };
    element = Sophus::SE3<T>(rotation, translation);
}

} // planning
