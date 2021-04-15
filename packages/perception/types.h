#pragma once

#include "Eigen/Eigen"

namespace perception {

typedef Eigen::Vector3f Point3f;

typedef struct {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    Eigen::MatrixXf xyz;
} PointCloudXYZ;

} // perception
