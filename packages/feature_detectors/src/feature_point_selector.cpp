
#include "packages/feature_detectors/include/feature_point_selector.h"
#include "packages/core/include/image_view.h"

#include <algorithm>
#include <cmath>

using namespace feature_detectors;

const std::vector<FeaturePoint>& FeaturePointSelector::select(const std::vector<FeaturePoint>& points) {
    m_points.clear();

    if (points.size() <= m_maxPoints) {
        return points;
    }

    for (auto& b1 : m_buckets) {
        for (auto& b2 : b1) {
            b2.clear();
        }
    }

    const size_t numBuckets = m_numBucketsRows * m_numBucketsCols;
    const size_t maxPointsPerBucket = m_maxPoints / numBuckets;
    const size_t dx = (size_t)std::ceil((float)m_cols / m_numBucketsCols);
    const size_t dy = (size_t)std::ceil((float)m_rows / m_numBucketsRows);

    // Place each point into a bucket
    for (size_t i = 0; i < points.size(); i++) {
        const int binX = points[i].x() / dx;
        const int binY = points[i].y() / dy;
        m_buckets[binX][binY].push_back(std::make_pair(points[i].score(), i));
    }

    // Visit each bucket and partial sort each bucket based on score
    for (auto& b1 : m_buckets) {
        for (auto& b2 : b1) {
            const size_t n = std::min<size_t>(b2.size(), maxPointsPerBucket);
            if (n == 0) {
                continue;
            }
            std::nth_element(b2.begin(), b2.begin() + n, b2.end(), std::greater<std::pair<float, size_t> >());
            for (size_t i = 0; i < n; i++) {
                m_points.push_back(points[b2[i].second]);
            }
        }
    }

    return m_points;
}
