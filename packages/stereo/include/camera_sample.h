#pragma once

#include "packages/filter_graph/include/aggregate_sample.h"
#include "packages/hal/proto/camera_sample.pb.h"

namespace stereo {
class CameraSample : public filter_graph::AggregateSample<hal::CameraSample> {
public:
    CameraSample(const std::string& streamId)
        : filter_graph::AggregateSample<hal::CameraSample>(streamId) {}
    ~CameraSample() = default;
};
}
