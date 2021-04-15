#pragma once

#include "packages/calibration/proto/coordinate_transformation.pb.h"
#include "packages/docking/proto/docking_station.pb.h"

namespace docking {

///
/// Subscribe poses of a fiducials on the docking station.
///
class FiducialPoseSourceInterface {
public:
    FiducialPoseSourceInterface() = default;
    ~FiducialPoseSourceInterface() = default;

    ///
    /// \brief Read poses of the fiducials for the target docking station
    /// \param poses    Poses of the associate fiducials
    /// \param dockingStation    The docking station meta data
    /// \return true : if the non-outdated data exist
    virtual bool readPoses(std::vector<calibration::CoordinateTransformation>& poses, const docking::DockingStation& dockingStation) = 0;
};
}