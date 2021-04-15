
#pragma once

#include "packages/feature_detectors/proto/feature_point.pb.h"

#include <vector>

namespace feature_detectors {

/// Selects the best feature points in an image.
class FeaturePointSelector {
public:
    typedef std::vector<std::pair<float, size_t> > bucket_t;

    FeaturePointSelector(
        const size_t rows, const size_t cols, const size_t numBucketsRows, const size_t numBucketsCols, const size_t maxPoints)
        : m_rows(rows)
        , m_cols(cols)
        , m_numBucketsRows(numBucketsRows)
        , m_numBucketsCols(numBucketsCols)
        , m_maxPoints(maxPoints) {
        m_buckets.resize(m_numBucketsCols);
        for (auto& b : m_buckets) {
            b.resize(m_numBucketsRows);
        }
    }
    ~FeaturePointSelector() = default;

    /// Select the best points in an image by placing a grid over the image and picking the best points in each bucket.
    /// \param points Source points that need to be selected from
    /// \return Vector of selected source points
    const std::vector<FeaturePoint>& select(const std::vector<FeaturePoint>& points);

    /// \return Vector of selected source points
    const std::vector<FeaturePoint>& points() const { return m_points; }

private:
    const size_t m_rows;
    const size_t m_cols;
    const size_t m_numBucketsRows;
    const size_t m_numBucketsCols;
    const size_t m_maxPoints;
    std::vector<std::vector<bucket_t> > m_buckets;
    std::vector<FeaturePoint> m_points;
};
}