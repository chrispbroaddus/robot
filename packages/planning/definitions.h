#pragma once

#include <cstdint>

namespace planning {
static constexpr int kXAxis = 0;
static constexpr int kYAxis = 1;
static constexpr int kZAxis = 2;

static constexpr int kDefaultPlannerToleranceNs = static_cast<int>(5e7);

template <typename T> struct PlannerConstants {
    static constexpr T kHardSpeedLimit = T(3) / T(2); // 1.5 m/s
    static constexpr T kCurvatureLimit = T(95) / T(100); // 0.95 (not sure about units)
    static constexpr size_t kMaxEntriesPerCommand = 31;
};

template <typename T> constexpr T PlannerConstants<T>::kHardSpeedLimit;

template <typename T> constexpr T PlannerConstants<T>::kCurvatureLimit;

template <typename T> constexpr size_t PlannerConstants<T>::kMaxEntriesPerCommand;

} // planning
