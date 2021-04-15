#pragma once

#include "packages/estimation/proto/estimator_options.pb.h"

namespace estimation {

EstimatorOptions loadDefaultEstimatorOptions();
WheelOdometryOptions loadDefaultWheelOdometryOptions();

} // estimation
