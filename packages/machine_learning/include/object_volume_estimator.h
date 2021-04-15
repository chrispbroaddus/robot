#pragma once

#include "glog/logging.h"
#include "packages/perception/proto/detection.pb.h"

namespace ml {

/// \brief Parameterization of the 3D volume, e.g., 3D bbox or convex hull
struct ObjectVolumeRepresentation {
    perception::Object3dBoundingBox box;
    perception::Object3dConvexHull convexHull;
};

/// \brief Input arguments for ObjectVolumeEstimator
struct ObjectVolumeEstimatorInput {
    perception::ObjectBoundingBox box;
    ObjectVolumeEstimatorInput(const perception::ObjectBoundingBox& newBox)
        : box(newBox) {}
};

/// \brief Base class for ObjectVolumeEstimator
class ObjectVolumeEstimator {
public:
    ObjectVolumeEstimator(const double minDepthExistancePerceptange, const size_t depthHistogramSize, const double maxPointCloudDistance,
        const size_t rgbImageCols, const size_t rgbImageRows, const calibration::KannalaBrandtRadialDistortionModel4<double>& kb4Model)
        : m_minDepthExistancePerceptange(minDepthExistancePerceptange)
        , m_depthHistogram(depthHistogramSize)
        , m_maxPointCloudDistance(maxPointCloudDistance)
        , m_rgbImageCols(rgbImageCols)
        , m_rgbImageRows(rgbImageRows)
        , m_kb4Model(kb4Model) {}

    virtual bool estimateObjectVolume(ObjectVolumeRepresentation& representation, const ObjectVolumeEstimatorInput& input,
        const std::vector<double>& depthMapOnImageSpace)
        = 0;

protected:
    double m_minDepthExistancePerceptange;
    std::vector<double> m_depthHistogram;
    uint32_t m_rgbImageCols;
    uint32_t m_rgbImageRows;
    double m_maxPointCloudDistance;
    const calibration::KannalaBrandtRadialDistortionModel4<double>& m_kb4Model;
};
}
