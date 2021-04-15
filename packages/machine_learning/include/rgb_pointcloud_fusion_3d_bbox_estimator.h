#pragma once

#include "object_volume_estimator.h"

#include "Eigen/Dense"
#include "glog/logging.h"

#include "packages/calibration/include/kannala_brandt_distortion_model.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/perception/proto/detection.pb.h"

namespace ml {

class RgbPointcloudFusion3dBboxEstimator : public ObjectVolumeEstimator {
public:
    RgbPointcloudFusion3dBboxEstimator(const double minDepthExistancePerceptange, const size_t depthHistogramSize,
        const double maxPointCloudDistance, const size_t rgbImageCols, const size_t rgbImageRows,
        const calibration::KannalaBrandtRadialDistortionModel4<double>& kb4Model)
        : ObjectVolumeEstimator(
              minDepthExistancePerceptange, depthHistogramSize, maxPointCloudDistance, rgbImageCols, rgbImageRows, kb4Model) {}

    bool estimateObjectVolume(
        ObjectVolumeRepresentation& box3d, const ObjectVolumeEstimatorInput& input, const std::vector<double>& depthMapOnImageSpace) {

        double depth3dBboxCenter;
        if (!estimateDepth3dBoundingBoxCenter(depth3dBboxCenter, input.box, depthMapOnImageSpace)) {
            return false;
        }

        calibration::CoordinateTransformation* pose = new calibration::CoordinateTransformation();
        CHECK_NOTNULL(pose);

        Eigen::Matrix<double, 2, 1> centerPoint;
        centerPoint << input.box.top_left_x() + input.box.extents_x() / 2, input.box.top_left_y() + input.box.extents_y() / 2;
        auto ray = m_kb4Model.unproject(centerPoint);
        CHECK_DOUBLE_EQ(ray(2), 1);
        double z = std::sqrt(depth3dBboxCenter * depth3dBboxCenter / (ray(0) * ray(0) + ray(1) * ray(1) + ray(2) * ray(2)));
        pose->set_translationx(ray(0) * z);
        pose->set_translationy(ray(1) * z);
        pose->set_translationz(ray(2) * z);

        perception::Category* objClass = new perception::Category();
        CHECK_NOTNULL(objClass);
        objClass->ParseFromString(input.box.category().SerializeAsString());

        box3d.box.set_allocated_category(objClass);
        box3d.box.set_allocated_pose(pose);

        const auto size = getAverageSize(input.box.category().type());
        box3d.box.set_extents_x(size.width);
        box3d.box.set_extents_y(size.height);
        box3d.box.set_extents_z(size.depth);

        return true;
    }

private:
    struct AverageSize {
        double height;
        double width;
        double depth;
        AverageSize(const double h, const double w, const double d)
            : height(h)
            , width(w)
            , depth(d) {}
        AverageSize(const double hwd[3])
            : height(hwd[0])
            , width(hwd[1])
            , depth(hwd[2]) {}
    };

    bool estimateDepth3dBoundingBoxCenter(
        double& modeDepth, const perception::ObjectBoundingBox& box, const std::vector<double>& depthMapOnImageSpace) {

        int xmin = box.top_left_x() + box.extents_x() / 4;
        int xmax = box.top_left_x() + box.extents_x() / 4 * 3;
        int ymin = box.top_left_y() + box.extents_y() / 4;
        int ymax = box.top_left_y() + box.extents_y() / 4 * 3;

        int nValidDepthPoint = 0;

        std::fill(m_depthHistogram.begin(), m_depthHistogram.end(), 0);

        for (int x = xmin; x < xmax; x++) {
            for (int y = ymin; y < ymax; y++) {

                if (!std::isinf(depthMapOnImageSpace[y * m_rgbImageCols + x]) && depthMapOnImageSpace[y * m_rgbImageCols + x] > 0) {
                    int index = (int)(depthMapOnImageSpace[y * m_rgbImageCols + x] * m_depthHistogram.size() / m_maxPointCloudDistance);
                    if (index >= 0 && index < m_depthHistogram.size()) {
                        m_depthHistogram[index]++;
                        nValidDepthPoint++;
                    }
                }
            }
        }

        if (nValidDepthPoint < box.extents_x() * box.extents_y() * m_minDepthExistancePerceptange) {
            return false;
        }

        auto result = std::max_element(m_depthHistogram.begin(), m_depthHistogram.end());
        auto histogramIndex = std::distance(m_depthHistogram.begin(), result);

        modeDepth = (double)histogramIndex / m_depthHistogram.size() * m_maxPointCloudDistance;

        // Add offset to get the enter of the object from the frontal surface
        const auto size = getAverageSize(box.category().type());
        modeDepth += size.depth / 2;

        return true;
    }

    // Default average size of objects, ordered by height, width, and depth, in meters
    const double PERSON_SIZE[3]{ 1.8, 0.6, 0.6 };
    const double CAR_SIZE[3]{ 1.4, 3, 3 };
    const double BICYCLE_SIZE[3]{ 1.2, 2, 2 };
    const double BUS_SIZE[3]{ 3, 3, 3 };
    const double TRUCK_SIZE[3]{ 3, 3, 3 };
    const double DEFAULT_SIZE[3] = { 0.1, 0.1, 0.1 };

    const AverageSize getAverageSize(const perception::Category_CategoryType& classType) {
        switch (classType) {
        case perception::Category::PERSON:
            return AverageSize(PERSON_SIZE);

        case perception::Category::CAR:
            return AverageSize(CAR_SIZE);

        case perception::Category::BICYCLE:
            return AverageSize(BICYCLE_SIZE);

        case perception::Category::BUS:
            return AverageSize(BUS_SIZE);

        case perception::Category::TRUCK:
            return AverageSize(TRUCK_SIZE);

        default:
            return AverageSize(DEFAULT_SIZE);
        }
    }
};
}
