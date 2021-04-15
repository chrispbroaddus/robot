#pragma once

#include <cmath>
#include <vector>

#include "calibu/Calibu.h"

#include "packages/calibration/proto/camera_intrinsic_calibration.pb.h"
#include "packages/calibration/proto/system_calibration.pb.h"
#include "packages/core/proto/geometry.pb.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/perception/grid.h"
#include "packages/perception/types.h"

namespace perception {

static constexpr float kMaxAllowableInvalidCloudPoints = 0.2;

bool imageToPointCloud(const hal::Image& image, core::PointCloud3d& cloud);

bool imageToPointCloud(const hal::Image& image, PointCloudXYZ& cloud);

void transform(PointCloudXYZ& cloud, const Sophus::SE3d& transform);

void checkCloud(const PointCloudXYZ& cloud, const int num_expected_points);

void saveCloud(const PointCloudXYZ& cloud);

calibration::SystemCalibration generateDummySystemCalibration();

calibration::SystemCalibration generateBasic4CameraSystemCalibration();

void cvToZippy(const Eigen::Vector3d& from, Eigen::Vector3d& to);

class UnprojectionLookup {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
public:
    UnprojectionLookup(std::shared_ptr<calibu::CameraInterface<double> > interface);

    const Eigen::ArrayXXf& lookup() const { return m_rayLookup; }

    void unproject(const hal::Image& image, PointCloudXYZ& cloud) const;

    void serialize(const PointCloudXYZ& in_cloud, core::PointCloud3d& out_cloud) const;

private:
    int x_dim, y_dim;
    Eigen::ArrayXXf m_rayLookup;
};

} // perception
