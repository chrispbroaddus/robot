#pragma once

#include "object_volume_estimator.h"

#include "Eigen/Dense"
#include "glog/logging.h"

#include "packages/calibration/include/kannala_brandt_distortion_model.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/math/geometry/convex_hull.h"
#include "packages/perception/proto/detection.pb.h"

namespace ml {

class RgbPointcloudFusion3dConvexHullEstimator : public ObjectVolumeEstimator {
public:
    RgbPointcloudFusion3dConvexHullEstimator(const double minDepthExistancePerceptange, const size_t depthHistogramSize,
        const double maxPointCloudDistance, const size_t rgbImageCols, const size_t rgbImageRows,
        const calibration::KannalaBrandtRadialDistortionModel4<double>& kb4Model)
        : ObjectVolumeEstimator(
              minDepthExistancePerceptange, depthHistogramSize, maxPointCloudDistance, rgbImageCols, rgbImageRows, kb4Model)
        , m_convexHullInlierThreshold(0.0) {}

    bool estimateObjectVolume(
        ObjectVolumeRepresentation& box3d, const ObjectVolumeEstimatorInput& input, const std::vector<double>& depthMapOnImageSpace) {

        double depth3dBboxCenter;
        std::vector<Eigen::Matrix<double, 3, 1> > points;

        if (!retrievePoints(depth3dBboxCenter, points, input.box, depthMapOnImageSpace)) {
            return false;
        }

        calibration::CoordinateTransformation* pose = new calibration::CoordinateTransformation();
        CHECK_NOTNULL(pose);

        Eigen::Matrix<double, 2, 1> centerPoint;
        centerPoint << input.box.top_left_x() + input.box.extents_x() / 2, input.box.top_left_y() + input.box.extents_y() / 2;
        auto ray = m_kb4Model.unproject(centerPoint);
        CHECK_DOUBLE_EQ(ray(2), 1);
        ray.normalize();
        ray *= depth3dBboxCenter;
        pose->set_translationx(ray(0));
        pose->set_translationy(ray(1));
        pose->set_translationz(ray(2));

        perception::Category* objClass = new perception::Category();
        CHECK_NOTNULL(objClass);
        objClass->ParseFromString(input.box.category().SerializeAsString());

        // Calculate convex hull
        std::vector<Eigen::Matrix<double, 2, 1> > point2ds;
        for (const auto& point : points) {
            Eigen::Matrix<double, 2, 1> m(point(0), point(2));
            point2ds.push_back(m);
        }

        int nConvexHullPoints = geometry::calculateConvexHullDestructive(point2ds);
        for (int i = 0; i < nConvexHullPoints; i++) {
            box3d.convexHull.add_xs(point2ds[i](0));
            box3d.convexHull.add_zs(point2ds[i](1));
        }

        box3d.convexHull.set_allocated_category(objClass);
        box3d.convexHull.set_allocated_pose(pose);

        const auto size = getAverageSize(input.box.category().type());
        box3d.convexHull.set_extents_y(size.height);

        return true;
    }

private:
    double m_convexHullInlierThreshold;

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

    bool retrievePoints(double& modeDepth, std::vector<Eigen::Matrix<double, 3, 1> >& points, const perception::ObjectBoundingBox& box,
        const std::vector<double>& depthMapOnImageSpace) {

        int xmin = box.top_left_x() + box.extents_x() / 16 * 3;
        int xmax = box.top_left_x() + box.extents_x() / 16 * 13;
        int ymin = box.top_left_y() + box.extents_y() / 4;
        int ymax = box.top_left_y() + box.extents_y() / 4 * 3;

        int nValidDepthPoint = 0;

        std::fill(m_depthHistogram.begin(), m_depthHistogram.end(), 0);

        std::vector<int> pointIndexInHistogram;
        std::vector<Eigen::Matrix<double, 3, 1> > pointCandidates;
        for (int x = xmin; x < xmax; x++) {
            for (int y = ymin; y < ymax; y++) {

                if (!std::isinf(depthMapOnImageSpace[y * m_rgbImageCols + x]) && depthMapOnImageSpace[y * m_rgbImageCols + x] > 0) {
                    int index = (int)(depthMapOnImageSpace[y * m_rgbImageCols + x] * m_depthHistogram.size() / m_maxPointCloudDistance);
                    if (index >= 0 && index < m_depthHistogram.size()) {
                        m_depthHistogram[index]++;
                        nValidDepthPoint++;

                        const Eigen::Matrix<double, 2, 1> m(x, y);
                        Eigen::Matrix<double, 3, 1> ray = m_kb4Model.unproject(m);

                        Eigen::Matrix<double, 2, 1> p = m_kb4Model.project(ray);
                        if (std::fabs(p(0) - x) > 1e-1 || std::fabs(p(1) - y) > 1e-1) {
                            continue;
                        }

                        ray.normalize();
                        ray *= depthMapOnImageSpace[y * m_rgbImageCols + x];

                        pointCandidates.push_back(ray);
                        pointIndexInHistogram.push_back(index);

                        // Augment extra depth from the object size prior
                        const auto size = getAverageSize(box.category().type());
                        double extendedDepth = depthMapOnImageSpace[y * m_rgbImageCols + x] + std::min(size.width, size.depth);
                        Eigen::Matrix<double, 3, 1> rayExtended = ray.normalized();
                        rayExtended *= extendedDepth;
                        pointCandidates.push_back(rayExtended);
                        pointIndexInHistogram.push_back(index);
                    }
                }
            }
        }

        if (nValidDepthPoint < box.extents_x() * box.extents_y() * m_minDepthExistancePerceptange) {
            return false;
        }

        auto result = std::max_element(m_depthHistogram.begin(), m_depthHistogram.end());
        auto histogramIndex = std::distance(m_depthHistogram.begin(), result);

        for (int i = 0; i < pointCandidates.size(); i++) {
            if (std::fabs(histogramIndex - pointIndexInHistogram[i]) / m_depthHistogram.size() * m_maxPointCloudDistance
                <= m_convexHullInlierThreshold) {
                points.push_back(pointCandidates[i]);
            }
        }
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
