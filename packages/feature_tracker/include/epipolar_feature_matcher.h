#pragma once

#include "packages/calibration/include/kannala_brandt_distortion_model.h"
#include "packages/calibration/proto/camera_intrinsic_calibration.pb.h"
#include "packages/core/include/image_view.h"
#include "packages/feature_tracker/include/epipolar_computation_utils.h"
#include "packages/feature_tracker/include/mutual_feature_match.h"
#include "packages/feature_tracker/include/ncc_feature_store.h"

#include "Eigen/Eigen"
#include "ncc_feature_store.h"

namespace feature_tracker {

/// EpipolarFeatureMatcher matches features detected in consecutive image frames in the temporal domain.
/// Match score is based off the NCC score and potential matches are determined using epipolar geometry
/// \tparam WINDOW_SIZE Size of the window used for NCC computation
/// \tparam T data-type of the matrix structures
template <size_t WINDOW_SIZE, typename T> class EpipolarFeatureMatcher {
public:
    /// Creates an epipolar feature matcher that matches features detected in the left and right images of a stereo pair
    /// \param leftIntrinsics left camera intrinsics
    /// \param rightIntrinsics right camera intrinsics
    /// \param rotation rotation matrix to go from the right camera to left camera coordinates
    /// \param translation translation vector to go from the rotated right camera to left camera coordinates
    /// \param maxPointDistance the max distance a point can be from the epipolar line to be considered as a potential match
    /// \param minMatchScore min NCC score required for a point to be considered as a potential match
    /// \param imageRows num of rows in the image
    /// \param imageCols num of columns in the image
    EpipolarFeatureMatcher(const calibration::CameraIntrinsicCalibration& leftIntrinsics,
        const calibration::CameraIntrinsicCalibration& rightIntrinsics, const Eigen::Matrix<T, 3, 3>& rotationMatrix,
        const Eigen::Matrix<T, 3, 1>& translationVector, float maxPointDistance, float minMatchScore, size_t imageRows, size_t imageCols);

    ~EpipolarFeatureMatcher() = default;
    EpipolarFeatureMatcher(const EpipolarFeatureMatcher&) = delete;
    EpipolarFeatureMatcher(const EpipolarFeatureMatcher&&) = delete;
    EpipolarFeatureMatcher& operator=(const EpipolarFeatureMatcher&) = delete;
    EpipolarFeatureMatcher& operator=(const EpipolarFeatureMatcher&&) = delete;

    /// Accepts NccFeatureStore data from a new left frame and sets up internal data-structures to match the new frame with the right frame
    /// \param newFeatureStore The NccFeatureStore extracted from the new left image frame
    void updateLeftFrame(const NccFeatureStore<WINDOW_SIZE>* newFeatureStore);

    /// Accepts NccFeatureStore data from a new right frame and sets up internal data-structures to match the new frame with the left frame
    /// \param newFeatureStore The NccFeatureStore extracted from the new right image frame
    void updateRightFrame(const NccFeatureStore<WINDOW_SIZE>* newFeatureStore);

    /// Computes the match pairs between features detected in the left and right frames
    /// \return vector of pairs of indices of matching feature points in the left and right frame
    /// The first element in the pair is the index of the feature point in left frame's list of detected features
    /// The second element in the pair is the index of the feature point in right frame's list of detected features that it matched to
    std::vector<std::pair<int, int> > matchFeatures();

private:
    /// Computes the NCC score on a pair of feature points
    /// \param featurePtr Pointer to a feature in the prev frame
    /// \param candidateFeaturePtr Pointer to a potential matching feature in the current frame
    /// \return NCC score
    float computeNccOnFeaturePair(const NccFeature<WINDOW_SIZE>& featurePtr, const NccFeature<WINDOW_SIZE>& candidateFeaturePtr);

    /// Finds the best match for a feature point in the left image from all the potential matches in the right frame
    void matchFeature(uint32_t featureIndex);

    /// Pointer to the feature store created using the current image frame.
    /// Non-owning pointer (Not used to allocate or deallocate new NccFeatureStores)
    /// The feature store should be in memory until the match function is called on the store
    const NccFeatureStore<WINDOW_SIZE>* m_leftFeatureStore;
    /// Left camera intrinsic camera matrix
    Eigen::Matrix<T, 3, 3> m_leftK;
    /// Left camera distortion coefficients
    Eigen::Matrix<T, 4, 1> m_leftDistortionCoef;
    /// Data-structure to store the point locations in calibrated space for the left frame
    std::vector<Eigen::Matrix<T, 3, 1> > m_leftCalibratedPoints;
    /// Data-structure to store the epipolar lines in the right image for feature points in the left image
    std::vector<Eigen::Matrix<T, 3, 1> > m_rightEpipolarLines;
    /// Data-structure to keep track of the best matches for all the feature points in the left frame
    std::vector<mutual_feature_match> m_leftMutualMatchFeature;
    /// Pointer to the feature store created using the previous image frame.
    /// Non-owning pointer (Not used to allocate or deallocate new NccFeatureStores)
    /// The feature store should be in memory until the match function is called on the store
    const NccFeatureStore<WINDOW_SIZE>* m_rightFeatureStore;
    /// Right camera intrinsic camera matrix
    Eigen::Matrix<T, 3, 3> m_rightK;
    /// Right camera distortion coefficients
    Eigen::Matrix<T, 4, 1> m_rightDistortionCoef;
    /// Data-structure to store the point locations in calibrated space for the previous frame
    std::vector<Eigen::Matrix<T, 3, 1> > m_rightCalibratedPoints;
    /// Data-structure to store the epipolar lines in the left image for feature points in the right image
    std::vector<Eigen::Matrix<T, 3, 1> > m_leftEpipolarLines;
    /// Data-structure to keep track of the best matches for all the feature points in the previous frame
    std::vector<mutual_feature_match> m_rightMutualMatchFeature;
    /// Maximum distance between feature points considered to be potential matches
    float m_maxPointDistance;
    /// Min nccScore required for a point to be considered as a match
    const float m_minMatchScore;
    /// Essential matrix between the left and right cameras
    Eigen::Matrix<T, 3, 3> m_essentialMatrix;
    /// Camera model to project and unproject points
    std::unique_ptr<calibration::KannalaBrandtRadialDistortionModel4<T> > m_leftCameraModel;
    /// Camera model to project and unproject points
    std::unique_ptr<calibration::KannalaBrandtRadialDistortionModel4<T> > m_rightCameraModel;
};

/// Convert camera intrinsics from protobuf format to eigen format
/// \tparam T data-type of the matrix structures
/// \param cameraIntrinsics Camera intrinsics in protobuf format
/// \param K camera matrix in Eigen::Matrix format
/// \param distortionCoef lens distortion in Eigen::Matrix format
template <typename T>
void inline cameraIntrinsicCalibrationToEigen(
    const calibration::CameraIntrinsicCalibration& cameraIntrinsics, Eigen::Matrix<T, 3, 3>& K, Eigen::Matrix<T, 4, 1>& distortionCoef) {

    if (!cameraIntrinsics.has_kannalabrandt()) {
        throw std::runtime_error("Intrinsics to Eigen matrix conversion failed: Unsupported distortion model");
    }

    K << cameraIntrinsics.scaledfocallengthx(), cameraIntrinsics.skew(), cameraIntrinsics.opticalcenterx(), 0,
        cameraIntrinsics.scaledfocallengthy(), cameraIntrinsics.opticalcentery(), 0, 0, 1;
    distortionCoef << cameraIntrinsics.kannalabrandt().radialdistortioncoefficientk(0),
        cameraIntrinsics.kannalabrandt().radialdistortioncoefficientk(1), cameraIntrinsics.kannalabrandt().radialdistortioncoefficientk(2),
        cameraIntrinsics.kannalabrandt().radialdistortioncoefficientk(3);
}

/// Creates a new feature matcher object and initializes it
/// \tparam WINDOW_SIZE Size of the window used for NCC computation
/// \tparam T data-type of the matrix structures
/// \param leftIntrinsics left camera intrinsics
/// \param rightIntrinsics right camera intrinsics
/// \param rotation rotation matrix to go from the right camera to left camera coordinates
/// \param translation translation vector to go from the rotated right camera to left camera coordinates
/// \param maxPointDistance the max distance a point can be from the epipolar line to be considered as a potential match
/// \param minMatchScore min NCC score required for a point to be considered as a potential match
/// \param imageRows num of rows in the image
/// \param imageCols num of columns in the image
template <size_t WINDOW_SIZE, typename T>
EpipolarFeatureMatcher<WINDOW_SIZE, T>::EpipolarFeatureMatcher(const calibration::CameraIntrinsicCalibration& leftIntrinsics,
    const calibration::CameraIntrinsicCalibration& rightIntrinsics, const Eigen::Matrix<T, 3, 3>& rotationMatrix,
    const Eigen::Matrix<T, 3, 1>& translationVector, float maxPointDistance, float minMatchScore, size_t imageRows, size_t imageCols)
    : m_leftFeatureStore(nullptr)
    , m_rightFeatureStore(nullptr)
    , m_maxPointDistance(maxPointDistance)
    , m_minMatchScore(minMatchScore) {

    if (m_maxPointDistance == 0) {
        throw std::runtime_error("Invalid max point distance: " + std::to_string(m_maxPointDistance));
    }

    m_essentialMatrix = computeEssentialMatrix(rotationMatrix, translationVector);

    cameraIntrinsicCalibrationToEigen(leftIntrinsics, m_leftK, m_leftDistortionCoef);
    cameraIntrinsicCalibrationToEigen(rightIntrinsics, m_rightK, m_rightDistortionCoef);

    if ((float)imageRows / leftIntrinsics.resolutiony() != (float)imageCols / leftIntrinsics.resolutionx()
        || (float)imageRows / rightIntrinsics.resolutiony() != (float)imageCols / rightIntrinsics.resolutionx()) {
        throw std::runtime_error("Image aspect ratio different from aspect ratio used for calibration");
    }
    float leftScale = (float)imageRows / leftIntrinsics.resolutiony();
    float rightScale = (float)imageRows / rightIntrinsics.resolutiony();
    m_leftK *= leftScale;
    m_leftK(2, 2) = 1.0;
    m_rightK *= rightScale;
    m_rightK(2, 2) = 1.0;
    m_maxPointDistance = maxPointDistance / m_leftK(0, 0);

    m_leftCameraModel.reset(new calibration::KannalaBrandtRadialDistortionModel4<T>(m_leftK, m_leftDistortionCoef, 10, 0));
    m_rightCameraModel.reset(new calibration::KannalaBrandtRadialDistortionModel4<T>(m_rightK, m_rightDistortionCoef, 10, 0));
}

/// Accepts NccFeatureStore data from a new left frame and sets up internal data-structures to match the new frame with the right frame
/// \param newFeatureStore The NccFeatureStore extracted from the new left image frame
template <size_t WINDOW_SIZE, typename T>
void EpipolarFeatureMatcher<WINDOW_SIZE, T>::updateLeftFrame(const NccFeatureStore<WINDOW_SIZE>* newFeatureStore) {

    /// Update left feature store
    m_leftFeatureStore = newFeatureStore;

    /// Resize the data-structures to be large enough to store values for all the feature points
    m_leftCalibratedPoints.resize(m_leftFeatureStore->getStoreSize());
    m_rightEpipolarLines.resize(m_leftFeatureStore->getStoreSize());

    /// Calculate the calibrated points for all the detected features in the left frame
    for (uint32_t featureIndex = 0; featureIndex < m_leftFeatureStore->getStoreSize(); featureIndex++) {
        Eigen::Matrix<T, 2, 1> point(m_leftFeatureStore->getX(featureIndex), m_leftFeatureStore->getY(featureIndex));
        m_leftCalibratedPoints[featureIndex] = m_leftCameraModel->unproject(point);
        m_rightEpipolarLines[featureIndex] = computeRightEpipolarLine(m_essentialMatrix, m_leftCalibratedPoints[featureIndex]);
    }
}

/// Accepts NccFeatureStore data from a new right frame and sets up internal data-structures to match the new frame with the left frame
/// \param newFeatureStore The NccFeatureStore extracted from the new right image frame
template <size_t WINDOW_SIZE, typename T>
void EpipolarFeatureMatcher<WINDOW_SIZE, T>::updateRightFrame(const NccFeatureStore<WINDOW_SIZE>* newFeatureStore) {

    /// Update right feature store
    m_rightFeatureStore = newFeatureStore;

    /// Resize the data-structures to be large enough to store values for all the feature points
    m_rightCalibratedPoints.resize(m_rightFeatureStore->getStoreSize());
    m_leftEpipolarLines.resize(m_rightFeatureStore->getStoreSize());

    /// Calculate the calibrated points for all the detected features in the right frame
    for (uint32_t featureIndex = 0; featureIndex < m_rightFeatureStore->getStoreSize(); featureIndex++) {
        Eigen::Matrix<T, 2, 1> point(m_rightFeatureStore->getX(featureIndex), m_rightFeatureStore->getY(featureIndex));
        m_rightCalibratedPoints[featureIndex] = m_rightCameraModel->unproject(point);
        m_leftEpipolarLines[featureIndex] = computeLeftEpipolarLine(m_essentialMatrix, m_rightCalibratedPoints[featureIndex]);
    }
}

/// Computes the NCC score on a pair of feature points
/// \param featurePtr Pointer to a feature in the left / right frame
/// \param candidateFeaturePtr Pointer to a potential matching feature in the right / left frame
/// \return NCC score
template <size_t WINDOW_SIZE, typename T>
float EpipolarFeatureMatcher<WINDOW_SIZE, T>::computeNccOnFeaturePair(
    const NccFeature<WINDOW_SIZE>& featurePtr, const NccFeature<WINDOW_SIZE>& candidateFeaturePtr) {

    /// Compute the NCC score on the feature pair using the precomputed NccFeatures
    /// imagePatch - Array containing image intensities of all the pixels in the window
    /// A - Sum of image intensities of all the pixels in the window
    /// B - Sum of squared intensities of all the pixels in the window
    /// C - 1/sqrt(nB - A^2) where n is the number of pixels in the window
    /// D - Dot product of the the two image patches
    /// NCC Score = (n*D - A1*A2)*C1*C2 where n is the number of pixels in the window
    float D = 0;
    for (uint32_t imageCtr = 0; imageCtr < WINDOW_SIZE * WINDOW_SIZE; imageCtr++) {
        D += *(featurePtr.imagePatch + imageCtr) * *(candidateFeaturePtr.imagePatch + imageCtr);
    }
    return (WINDOW_SIZE * WINDOW_SIZE * D - featurePtr.A * candidateFeaturePtr.A) * featurePtr.C * candidateFeaturePtr.C;
}

/// Computes the match score between the specified feature point in the previous frame with all the valid feature points
/// in the current frame
/// \param gridRow The row number of the grid location where the feature is located
/// \param gridCol The col number of the grid location where the feature is located
/// \param bucketIndex The index of the feature within the bucket at the specified grid location
template <size_t WINDOW_SIZE, typename T> void EpipolarFeatureMatcher<WINDOW_SIZE, T>::matchFeature(uint32_t featureIndex) {

    /// Get the nccFeaturePtr for the point that we want to match using the feature index
    const NccFeature<WINDOW_SIZE>* nccFeaturePtr = m_leftFeatureStore->getNccFeaturePtr(featureIndex);
    /// Get the point in calibrated space corresponding to the point we want to match using the feature index
    Eigen::Matrix<T, 3, 1> calibratedPoint = m_leftCalibratedPoints[featureIndex];

    /// Loop over the all the features and find potential matches by computing their distance from the epipolar line
    const NccFeature<WINDOW_SIZE>* candidateFeaturePtr;
    for (uint32_t featureCtr = 0; featureCtr < m_rightCalibratedPoints.size(); featureCtr++) {

        /// Get the candidate calibrated point and its NccFeature data
        Eigen::Matrix<T, 3, 1> candidateCalibratedPoint = m_rightCalibratedPoints[featureCtr];
        candidateFeaturePtr = m_rightFeatureStore->getNccFeaturePtr(featureCtr);

        /// Compute the distance from the candidate point to the epipolar line corresponding to the point being matched
        float distance = shortestDistPointToLine(candidateCalibratedPoint, m_rightEpipolarLines[featureIndex]);
        float nccScore = 0;
        bool computedNccScore = false;
        if (distance < m_maxPointDistance) {
            /// Compute the NCC score if the distance requirement is met
            nccScore = computeNccOnFeaturePair(*nccFeaturePtr, *candidateFeaturePtr);
            computedNccScore = true;
            if (nccScore > m_minMatchScore) {
                /// Update best match if distance and NCC score criteria are met
                m_leftMutualMatchFeature[featureIndex].featureIndex = featureIndex;
                m_leftMutualMatchFeature[featureIndex].updateBestMatch(&m_rightMutualMatchFeature[featureCtr], nccScore);
            }
        }

        /// Compute the distance of the point being matched from the epipolar line of the candidate point
        distance = shortestDistPointToLine(calibratedPoint, m_leftEpipolarLines[featureCtr]);
        if (distance < m_maxPointDistance) {
            if (!computedNccScore) {
                /// Compute the NCC score if the distance requirement is met and has not already been computed
                nccScore = computeNccOnFeaturePair(*nccFeaturePtr, *candidateFeaturePtr);
            }
            if (nccScore > m_minMatchScore) {
                /// Update best match if distance and NCC score criteria are met
                m_rightMutualMatchFeature[featureCtr].featureIndex = featureCtr;
                m_rightMutualMatchFeature[featureCtr].updateBestMatch(&m_leftMutualMatchFeature[featureIndex], nccScore);
            }
        }
    }
}

/// Computes the match pairs between features detected in the previous and current frames
/// \return vector of pairs of indices of matching feature points in the previous and current frame
/// The first element in the pair is the index of the feature point in previous frame's feature store
/// The second element in the pair is the index of the feature point in current frame's feature store that it matched to
template <size_t WINDOW_SIZE, typename T> std::vector<std::pair<int, int> > EpipolarFeatureMatcher<WINDOW_SIZE, T>::matchFeatures() {
    /// The matching algorithm should be run only after at least two images have been received
    if (m_leftFeatureStore == nullptr || m_rightFeatureStore == nullptr) {
        return std::vector<std::pair<int, int> >();
    }

    /// Clear MutualMatchFeatures
    m_leftMutualMatchFeature.clear();
    m_leftMutualMatchFeature.resize(m_leftFeatureStore->getStoreSize());
    m_rightMutualMatchFeature.clear();
    m_rightMutualMatchFeature.resize(m_rightFeatureStore->getStoreSize());

    /// Loop over all the features in the left frame and look for matches in the right frame
    for (uint32_t featureCtr = 0; featureCtr < m_leftFeatureStore->getStoreSize(); featureCtr++) {
        matchFeature(featureCtr);
    }

    /// Create a vector of pairs with the indices of the feature points that have been matched
    /// The first element in the pair is the index of the feature point in previous frame's feature store
    /// The second element in the pair is the index of the feature point in current frame's feature store that it matched to
    std::vector<std::pair<int, int> > matchedIndices;
    for (uint32_t featureCtr = 0; featureCtr < m_leftMutualMatchFeature.size(); featureCtr++) {
        if (m_leftMutualMatchFeature[featureCtr].isMutuallyBest()) {
            matchedIndices.push_back(std::make_pair(m_leftFeatureStore->getFeaturePointIndex(featureCtr),
                m_rightFeatureStore->getFeaturePointIndex(m_leftMutualMatchFeature[featureCtr].bestMatch->featureIndex)));
        }
    }
    return matchedIndices;
}
}
