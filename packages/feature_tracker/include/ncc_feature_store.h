#pragma once

#include "packages/core/include/image_view.h"
#include "packages/feature_detectors/proto/feature_point.pb.h"

#include <cmath>

namespace feature_tracker {

/// @brief NccFeature is a data-structure used to store all the data required to compute Normalized Cross-Correlation
/// x, y - coordinates of the feature point in the image
/// A - Sum of image intensities of all the pixels in the window
/// B - Sum of squared intensities of all the pixels in the window
/// C - 1/sqrt(nB - A^2) where n is the number of pixels in the window
/// imagePatch - Array containing image intensities of all the pixels in the window
/// \tparam WINDOW_SIZE Size of the window used for NCC computation
template <size_t WINDOW_SIZE> struct NccFeature {

    NccFeature() {
        x = 0;
        y = 0;
        A = 0;
        B = 0;
        C = 0;
        featurePointIndex = -1;
    }

    float x;
    float y;
    uint32_t A;
    uint32_t B;
    float C;
    uint8_t imagePatch[WINDOW_SIZE * WINDOW_SIZE];
    int featurePointIndex;
};

/// NccFeatureStore pre-computes and stores data required to compute Normalized Cross-Correlation at
/// detected feature points
/// \tparam WINDOW_SIZE Size of the window used for NCC computation
template <size_t WINDOW_SIZE> class NccFeatureStore {
public:
    /// Creates an NccFeatureStore with NccFeature structures for all the valid feature points detected in the image
    /// \param image Image to be used for calculating the NCC coefficients
    /// \param featurePoints Feature points detected on the image
    NccFeatureStore(const core::ImageView<core::ImageType::uint8>& image, const std::vector<feature_detectors::FeaturePoint>& featurePoints)
        : m_imageRows(image.rows)
        , m_imageCols(image.cols) {
        computeNccFeatures(image, featurePoints);
    }

    /// Creates a NccFeatureStore from pre-populated NccFeatures. Used for tests
    /// \param nccFeatures
    NccFeatureStore(const std::vector<NccFeature<WINDOW_SIZE> >& nccFeatures, const size_t& imageRows, const size_t& imageCols)
        : m_imageRows(imageRows)
        , m_imageCols(imageCols) {
        m_nccFeatures = nccFeatures;
    }

    NccFeatureStore(const NccFeatureStore&) = delete;
    NccFeatureStore(const NccFeatureStore&&) = delete;
    NccFeatureStore& operator=(const NccFeatureStore&) = delete;
    NccFeatureStore& operator=(const NccFeatureStore&&) = delete;

    /// Get the window size being used for NCC computations
    /// \param index
    /// \return ncc coefficient A
    size_t getWindowSize() const { return m_windowSize; }

    /// Get the number of NccFeatures in the store
    /// \return size of the vector of NccFeatures
    size_t getStoreSize() const { return m_nccFeatures.size(); }

    /// Get the number of rows in the image used to generate the NccFeatureStore
    /// \return number of rows in the image
    uint32_t getImageRows() const { return m_imageRows; }

    /// Get the number of cols in the image used to generate the NccFeatureStore
    /// \return number of cols in the image
    uint32_t getImageCols() const { return m_imageCols; }

    /// Get the NccFeature structure for the feature stored at the given index
    /// \param index
    /// \return pointer to the NccFeature at index
    const NccFeature<WINDOW_SIZE>* getNccFeaturePtr(int index) const { return m_nccFeatures.data() + index; }

    /// Get the x coordinate of the feature point stored at the given index
    /// \param index
    /// \return x coordinate
    float getX(int index) const { return m_nccFeatures[index].x; }

    /// Get the Y coordinate of the feature point stored at the given index
    /// \param index
    /// \return y coordinate
    float getY(int index) const { return m_nccFeatures[index].y; }

    /// Get the index of the featurePoint in the featurePoints vector that the NccFeature stored at the given index corresponds to
    /// \param index
    /// \return index of featurePoint in the featurePoints vector that the NccFeature corresponds to
    int getFeaturePointIndex(int index) const { return m_nccFeatures[index].featurePointIndex; }

    /// Get the value of coefficient A(see above) for the feature point stored at the given index
    /// \param index
    /// \return ncc coefficient A
    uint32_t getA(int index) const { return m_nccFeatures[index].A; }

    /// Get the value of coefficient B(see above) for the feature point stored at the given index
    /// \param index
    /// \return ncc coefficient B
    uint32_t getB(int index) const { return m_nccFeatures[index].B; }

    /// Get the value of coefficient C(see above) for the feature point stored at the given index
    /// \param index
    /// \return ncc coefficient C
    float getC(float index) const { return m_nccFeatures[index].C; }

    /// Get the pointer to head of the image patch around the feature point stored at the given index
    /// \param index
    /// \return ncc coefficient A
    const uint8_t* getImagePatch(float index) const { return m_nccFeatures[index].imagePatch; }

private:
    /// Computes NccFeatures for all valid feature points and stores them in m_nccFeatures
    /// \param image Image to be used for calculating the NCC coefficients
    /// \param featurePoints Feature points detected on the image
    void computeNccFeatures(
        const core::ImageView<core::ImageType::uint8>& image, const std::vector<feature_detectors::FeaturePoint>& featurePoints);

    const size_t m_windowSize = WINDOW_SIZE;
    const uint32_t m_imageRows;
    const uint32_t m_imageCols;
    std::vector<NccFeature<WINDOW_SIZE> > m_nccFeatures;
};

/// Computes an NccFeature for a single feature point
template <size_t WINDOW_SIZE>
void inline computeNccFeature(const feature_detectors::FeaturePoint* featurePoint, const uint8_t* imageData, size_t stride,
    int featurePointIndex, struct NccFeature<WINDOW_SIZE>& nccFeature) {
    /// Compute NccFeature data
    nccFeature.x = featurePoint->x();
    nccFeature.y = featurePoint->y();
    nccFeature.featurePointIndex = featurePointIndex;
    uint8_t* imagePatchData = nccFeature.imagePatch;
    constexpr uint32_t halfWindowSize = (uint32_t)WINDOW_SIZE / 2;

    /// Loop over the pixels in the window and build the NccFeature structure
    for (uint32_t rowCtr = 0; rowCtr < WINDOW_SIZE; rowCtr++) {
        const uint8_t* rowPtr
            = imageData + ((int)featurePoint->y() - halfWindowSize + rowCtr) * stride + ((int)featurePoint->x() - halfWindowSize);
        for (uint32_t colCtr = 0; colCtr < WINDOW_SIZE; colCtr++) {
            nccFeature.A += *(rowPtr + colCtr);
            nccFeature.B += (*(rowPtr + colCtr)) * (*(rowPtr + colCtr));
            *imagePatchData++ = *(rowPtr + colCtr);
        }
        nccFeature.C = 1.0f / (float)std::sqrt(WINDOW_SIZE * WINDOW_SIZE * nccFeature.B - nccFeature.A * nccFeature.A);
    }
}

/// Computes NccFeatures for all valid feature points and stores them in m_nccFeatures
template <size_t WINDOW_SIZE>
void NccFeatureStore<WINDOW_SIZE>::computeNccFeatures(
    const core::ImageView<core::ImageType::uint8>& image, const std::vector<feature_detectors::FeaturePoint>& featurePoints) {

    /// Input sanity checks
    if (image.rows == 0 || image.cols == 0 || image.stride == 0 || image.data == nullptr || image.stride < image.cols) {
        throw std::runtime_error("Invalid input image");
    }
    if (WINDOW_SIZE % 2 == 0) {
        throw std::runtime_error("Invalid window size, window size needs to be odd");
    }

    constexpr uint32_t halfWindowSize = (uint32_t)WINDOW_SIZE / 2;
    const uint8_t* imageData = image.data;
    const size_t rows = image.rows;
    const size_t cols = image.cols;
    const size_t stride = image.stride;

    /// Loop over all the detected points and build the vector of NccFeatures
    m_nccFeatures.reserve(featurePoints.size());
    const feature_detectors::FeaturePoint* featurePoint = nullptr;
    for (uint32_t featureCtr = 0; featureCtr < featurePoints.size(); featureCtr++) {
        featurePoint = featurePoints.data() + featureCtr;
        /// Ignore feature points on the border
        if (featurePoint->x() < halfWindowSize || featurePoint->x() >= cols - halfWindowSize || featurePoint->y() < halfWindowSize
            || featurePoint->y() >= rows - halfWindowSize) {
            continue;
        }
        struct NccFeature<WINDOW_SIZE> nccFeature;
        computeNccFeature<WINDOW_SIZE>(featurePoint, imageData, stride, featureCtr, nccFeature);
        m_nccFeatures.push_back(std::move(nccFeature));
    }
    m_nccFeatures.shrink_to_fit();
}
} // feature_tracker
