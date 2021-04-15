
#pragma once

#include "packages/calibration/proto/camera_intrinsic_calibration.pb.h"
#include "packages/feature_tracker/include/epipolar_feature_matcher.h"
#include "packages/feature_tracker/include/ncc_feature_matcher.h"

namespace feature_tracker {
/// Stereo matcher based on epipolar geometry, NCC and mutual correspondence.
template <size_t WINDOW_SIZE, typename T> class StereoFeatureMatcher {
public:
    StereoFeatureMatcher(const float maxPointDistance, const float minNccScore,
        const calibration::CameraIntrinsicCalibration& leftCameraIntrinsics,
        const calibration::CameraIntrinsicCalibration& rightCameraIntrinsics, const Eigen::Matrix<T, 3, 3>& rotationMatrix,
        const Eigen::Matrix<T, 3, 1>& translationVector)
        : m_maxPointDistance(maxPointDistance)
        , m_minNccScore(minNccScore)
        , m_leftCameraIntrinsics(leftCameraIntrinsics)
        , m_rightCameraIntrinsics(rightCameraIntrinsics)
        , m_rotationMatrix(rotationMatrix)
        , m_translationVector(translationVector) {}
    ~StereoFeatureMatcher() = default;

    /// Match the feature stores from left/right images.
    /// \param leftFeatureStore The left feature store
    /// \param rightFeatureStore The right feature store
    /// \return Vector of left/right indices mapping back into the feature stores
    std::vector<std::pair<int, int> > matchFeatureStores(
        const NccFeatureStore<WINDOW_SIZE>& leftFeatureStore, const NccFeatureStore<WINDOW_SIZE>& rightFeatureStore) {
        if (leftFeatureStore.getImageRows() != rightFeatureStore.getImageRows()
            || leftFeatureStore.getImageCols() != rightFeatureStore.getImageCols()) {
            throw std::runtime_error("feature store image sizes don't match");
        }
        EpipolarFeatureMatcher<WINDOW_SIZE, T> matcher(m_leftCameraIntrinsics, m_rightCameraIntrinsics, m_rotationMatrix,
            m_translationVector, m_maxPointDistance, m_minNccScore, leftFeatureStore.getImageRows(), leftFeatureStore.getImageCols());
        matcher.updateLeftFrame(&leftFeatureStore);
        matcher.updateRightFrame(&rightFeatureStore);
        return matcher.matchFeatures();
    };

private:
    const float m_maxPointDistance;
    const float m_minNccScore;
    const calibration::CameraIntrinsicCalibration m_leftCameraIntrinsics;
    const calibration::CameraIntrinsicCalibration m_rightCameraIntrinsics;
    const Eigen::Matrix<T, 3, 3> m_rotationMatrix;
    const Eigen::Matrix<T, 3, 1> m_translationVector;
};
}
