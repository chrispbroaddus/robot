#pragma once

#include "sophus/se3.hpp"

namespace estimation {
template <typename T> void poseToProto(const Sophus::SE3<T>& in, estimation::State& out);
}

#include "utils.hpp"
