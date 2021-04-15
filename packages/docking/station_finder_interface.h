#pragma once

#include "packages/docking/proto/docking_station.pb.h"
#include <atomic>

namespace docking {

///
/// The abstract interface for docking-station database server
/// It returns the list of docking-stations given its query value
///
class StationFinderInterface {
public:
    StationFinderInterface() = default;
    ~StationFinderInterface() = default;

    virtual void publish(
        const std::string& serverAddress, const int highWaterMark, const int lingerPeriodInMilliseconds, std::atomic_bool& stop)
        = 0;
};
}