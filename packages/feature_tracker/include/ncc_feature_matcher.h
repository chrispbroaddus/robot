#pragma once

#include "packages/core/include/image_view.h"
#include "packages/feature_tracker/include/mutual_feature_match.h"
#include "packages/feature_tracker/include/ncc_feature_store.h"

namespace feature_tracker {

/// NccFeatureMatcher matches features detected in consecutive image frames in the temporal domain.
/// Match score is based off the NCC score
/// \tparam WINDOW_SIZE Size of the window used for NCC computation
template <size_t WINDOW_SIZE> class NccFeatureMatcher {
public:
    using bucket_type = std::vector<uint32_t>;

    NccFeatureMatcher(uint32_t searchRadius, uint32_t imageRows, uint32_t imageCols, float minNccScore);
    ~NccFeatureMatcher();

    NccFeatureMatcher(const NccFeatureMatcher&) = delete;
    NccFeatureMatcher(const NccFeatureMatcher&&) = delete;
    NccFeatureMatcher& operator=(const NccFeatureMatcher&) = delete;
    NccFeatureMatcher& operator=(const NccFeatureMatcher&&) = delete;

    /// Accepts NccFeatureStore data from a new frame and sets up internal data-structures to match the new frame with the previous one
    /// \param newFeatureStore The NccFeatureStore extracted from the current image frame
    void updateCurrentFrame(const NccFeatureStore<WINDOW_SIZE>* newFeatureStore);

    /// Computes the match pairs between features detected in the previous and current frames
    /// \return vector of pairs of indices of matching feature points in the previous and current frame
    /// The first element in the pair is the index of the feature point in previous frame's feature store
    /// The second element in the pair is the index of the feature point in current frame's feature store that it matched to
    std::vector<std::pair<int, int> > matchFeatures();

    /// Get the size of the grid the matcher uses to create buckets of feature points
    /// \return grid size
    uint32_t getGridSize() { return m_gridSize; }

    /// Get the number of rows in the grid
    /// \return number of rows in the grid
    uint32_t getNumGridRows() { return m_numGridRows; }

    /// Get the number of cols in the grid
    /// \return number of cols in the grid
    uint32_t getNumGridCols() { return m_numGridCols; }

private:
    /// Computes the NCC score on a pair of feature points
    /// \param featurePtr Pointer to a feature in the prev frame
    /// \param candidateFeaturePtr Pointer to a potential matching feature in the current frame
    /// \return NCC score
    float computeNccOnFeaturePair(const NccFeature<WINDOW_SIZE>& featurePtr, const NccFeature<WINDOW_SIZE>& candidateFeaturePtr);

    /// Computes the match score between the specified feature point in the previous frame with all the valid feature points
    /// in the current frame
    /// \param gridRow The row number of the grid location where the feature is located
    /// \param gridCol The col number of the grid location where the feature is located
    /// \param bucketIndex The index of the feature within the bucket at the specified grid location
    void matchFeature(int gridRow, int gridCol, uint32_t bucketIndex);

    /// Pointer to the feature store created using the current image frame.
    /// Non-owning pointer (Not used to allocate or deallocate new MutualMatchFeatures)
    /// The feature store should be in memory until the match function is called on the store with the previous and following frames
    const NccFeatureStore<WINDOW_SIZE>* m_currentFeatureStore;
    /// Data-structure to keep track feature points in the current frame after dividing them into a 2-D grid based on search radius
    std::vector<std::vector<bucket_type> > m_currentBuckets;
    /// Data-structure to keep track of the best matches for all the feature points in the current frame
    std::vector<mutual_feature_match> m_currentMutualMatchFeature;
    /// Pointer to the feature store created using the previous image frame.
    /// Non-owning pointer (Not used to allocate or deallocate new MutualMatchFeatures)
    /// The feature store should be in memory until the match function is called on the store with the previous and following frames
    const NccFeatureStore<WINDOW_SIZE>* m_prevFeatureStore;
    /// Data-structure to keep track feature points in the previous frame after dividing them into a 2-D grid based on search radius
    std::vector<std::vector<bucket_type> > m_prevBuckets;
    /// Data-structure to keep track of the best matches for all the feature points in the previous frame
    std::vector<mutual_feature_match> m_prevMutualMatchFeature;
    /// Maximum distance between feature points considered to be potential matches
    const uint32_t m_searchRadius;
    /// Size of the 2D grid that the feature points are divided into
    const uint32_t m_gridSize;
    /// Number of rows in the grid
    const uint32_t m_numGridRows;
    /// Number of columns in the grid
    const uint32_t m_numGridCols;
    /// The minimum NCC that is considered as a valid match
    const float m_minNccScore;
};

/// Creates a new feature matcher object and initializes it
/// \tparam WINDOW_SIZE Size of the window used for NCC computation
/// \param searchRadius The max distance in pixels to search for a potential match
/// \param imageRows Number of rows in the image
/// \param imageCols Number of cols in the image
/// \param minNccScore Minimum valid NCC score
template <size_t WINDOW_SIZE>
NccFeatureMatcher<WINDOW_SIZE>::NccFeatureMatcher(uint32_t searchRadius, uint32_t imageRows, uint32_t imageCols, float minNccScore)
    : m_currentFeatureStore(nullptr)
    , m_prevFeatureStore(nullptr)
    , m_searchRadius(searchRadius)
    , m_gridSize(2 * m_searchRadius)
    , m_numGridRows((uint32_t)std::ceil((float)imageRows / m_gridSize))
    , m_numGridCols((uint32_t)std::ceil((float)imageCols / m_gridSize))
    , m_minNccScore(minNccScore) {

    /// Input sanity checks
    if (imageRows == 0 || imageCols == 0) {
        throw std::runtime_error("Invalid image size");
    }
    if (searchRadius == 0) {
        throw std::runtime_error("Invalid search radius");
    }
}

/// Accepts NccFeatureStore data from a new frame and sets up internal data-structures to match the new frame with the previous one
/// \param newFeatureStore The NccFeatureStore extracted from the current image frame
template <size_t WINDOW_SIZE> void NccFeatureMatcher<WINDOW_SIZE>::updateCurrentFrame(const NccFeatureStore<WINDOW_SIZE>* newFeatureStore) {

    /// Update previous and current frames
    m_prevFeatureStore = m_currentFeatureStore;
    m_currentFeatureStore = newFeatureStore;

    /// Update previous bucket and reuse previous frame's bucket structure for the current frame
    m_prevBuckets.swap(m_currentBuckets);

    /// Clear the feature indices from the buckets if an old bucket is being reused, else create a new one
    if (!m_currentBuckets.empty()) {
        for (auto& b1 : m_currentBuckets) {
            for (auto& b2 : b1) {
                b2.clear();
            }
        }
    } else {
        m_currentBuckets.resize(m_numGridRows);
        for (auto& row : m_currentBuckets) {
            row.resize(m_numGridCols);
        }
    }

    /// Update current frame's bucket
    for (uint32_t featureIndex = 0; featureIndex < m_currentFeatureStore->getStoreSize(); featureIndex++) {
        const uint32_t row = (uint32_t)m_currentFeatureStore->getY(featureIndex) / m_gridSize;
        const uint32_t col = (uint32_t)m_currentFeatureStore->getX(featureIndex) / m_gridSize;
        m_currentBuckets[row][col].push_back(featureIndex);
    }
}

/// Computes the NCC score on a pair of feature points
/// \param featurePtr Pointer to a feature in the prev frame
/// \param candidateFeaturePtr Pointer to a potential matching feature in the current frame
/// \return NCC score
template <size_t WINDOW_SIZE>
float NccFeatureMatcher<WINDOW_SIZE>::computeNccOnFeaturePair(
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
template <size_t WINDOW_SIZE> void NccFeatureMatcher<WINDOW_SIZE>::matchFeature(int gridRow, int gridCol, uint32_t bucketIndex) {

    /// Get the NccFeature data for the specified feature point using the index stored in the bucket
    uint32_t featureIndex = m_prevBuckets[gridRow][gridCol][bucketIndex];
    const NccFeature<WINDOW_SIZE>* featurePtr = m_prevFeatureStore->getNccFeaturePtr(featureIndex);

    /// Get the indices of the buckets where potential matches can lie
    uint32_t searchGridRowStart;
    uint32_t searchGridRowEnd;
    uint32_t searchGridColStart;
    uint32_t searchGridColEnd;
    if (featurePtr->y / m_gridSize - gridRow >= 0.5f) {
        searchGridRowStart = (uint32_t)gridRow;
        searchGridRowEnd = gridRow + 1 < (int)m_numGridRows - 1 ? (uint32_t)gridRow + 1 : m_numGridRows - 1;
    } else {
        searchGridRowStart = (gridRow - 1) > 0 ? (uint32_t)gridRow - 1 : 0;
        searchGridRowEnd = (uint32_t)gridRow;
    }
    if (featurePtr->x / m_gridSize - gridCol >= 0.5f) {
        searchGridColStart = (uint32_t)gridCol;
        searchGridColEnd = gridCol + 1 < (int)m_numGridCols - 1 ? (uint32_t)gridCol + 1 : m_numGridCols - 1;
    } else {
        searchGridColStart = gridCol - 1 > 0 ? (uint32_t)gridCol - 1 : 0;
        searchGridColEnd = (uint32_t)gridCol;
    }

    /// Loop over the all the features in the buckets where potential matches can lie and compute the Ncc score with those features
    uint32_t candidateFeatureIndex;
    const NccFeature<WINDOW_SIZE>* candidateFeaturePtr;
    float squaredRadius = m_searchRadius * m_searchRadius;
    for (uint32_t rowCtr = searchGridRowStart; rowCtr <= searchGridRowEnd; rowCtr++) {
        for (uint32_t colCtr = searchGridColStart; colCtr <= searchGridColEnd; colCtr++) {
            for (uint32_t bucketCtr = 0; bucketCtr < m_currentBuckets[rowCtr][colCtr].size(); bucketCtr++) {
                /// Get NccFeature data for the candidate feature point using the index stored in the bucket
                candidateFeatureIndex = m_currentBuckets[rowCtr][colCtr][bucketCtr];
                candidateFeaturePtr = m_currentFeatureStore->getNccFeaturePtr(candidateFeatureIndex);
                /// Compute NCC score if the candidate feature point meets the distance requirement
                const float squaredDistance = (featurePtr->x - candidateFeaturePtr->x) * (featurePtr->x - candidateFeaturePtr->x)
                    + (featurePtr->y - candidateFeaturePtr->y) * (featurePtr->y - candidateFeaturePtr->y);
                if (squaredDistance <= squaredRadius) {
                    const float nccScore = computeNccOnFeaturePair(*featurePtr, *candidateFeaturePtr);
                    if (nccScore >= m_minNccScore) {
                        /// Update the MutualMatchFeature data-structure
                        m_prevMutualMatchFeature[featureIndex].featureIndex = featureIndex;
                        m_prevMutualMatchFeature[featureIndex].updateBestMatch(
                            &m_currentMutualMatchFeature[candidateFeatureIndex], nccScore);
                        m_currentMutualMatchFeature[candidateFeatureIndex].featureIndex = candidateFeatureIndex;
                        m_currentMutualMatchFeature[candidateFeatureIndex].updateBestMatch(
                            &m_prevMutualMatchFeature[featureIndex], nccScore);
                    }
                }
            }
        }
    }
}

/// Computes the match pairs between features detected in the previous and current frames
/// \return vector of pairs of indices of matching feature points in the previous and current frame
/// The first element in the pair is the index of the feature point in previous frame's feature store
/// The second element in the pair is the index of the feature point in current frame's feature store that it matched to
template <size_t WINDOW_SIZE> std::vector<std::pair<int, int> > NccFeatureMatcher<WINDOW_SIZE>::matchFeatures() {

    /// The matching algorithm should be run only after at least two images have been received
    if (m_prevFeatureStore == nullptr) {
        return std::vector<std::pair<int, int> >();
    }

    /// Clear MutualMatchFeatures
    m_currentMutualMatchFeature.clear();
    m_currentMutualMatchFeature.resize(m_currentFeatureStore->getStoreSize());
    m_prevMutualMatchFeature.clear();
    m_prevMutualMatchFeature.resize(m_prevFeatureStore->getStoreSize());

    /// Loop over all the features in all the buckets of the previous frame and look for matches in the current frame
    for (uint32_t rowCtr = 0; rowCtr < m_numGridRows; rowCtr++) {
        for (uint32_t colCtr = 0; colCtr < m_numGridCols; colCtr++) {
            for (uint32_t bucketCtr = 0; bucketCtr < m_prevBuckets[rowCtr][colCtr].size(); bucketCtr++) {
                matchFeature(rowCtr, colCtr, bucketCtr);
            }
        }
    }

    /// Create a vector of pairs with the indices of the feature points that have been matched
    /// The first element in the pair is the index of the feature point in previous frame's feature store
    /// The second element in the pair is the index of the feature point in current frame's feature store that it matched to
    std::vector<std::pair<int, int> > matchedIndices;
    for (uint32_t featureCtr = 0; featureCtr < m_prevMutualMatchFeature.size(); featureCtr++) {
        if (m_prevMutualMatchFeature[featureCtr].isMutuallyBest()) {
            matchedIndices.push_back(std::make_pair(m_prevFeatureStore->getFeaturePointIndex(featureCtr),
                m_currentFeatureStore->getFeaturePointIndex(m_prevMutualMatchFeature[featureCtr].bestMatch->featureIndex)));
        }
    }
    return matchedIndices;
}

template <size_t WINDOW_SIZE> NccFeatureMatcher<WINDOW_SIZE>::~NccFeatureMatcher() {}
}
