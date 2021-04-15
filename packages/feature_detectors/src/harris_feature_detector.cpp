
#include "packages/feature_detectors/include/harris_feature_detector.h"
#include "packages/feature_detectors/include/harris_feature_detector_details.h"

using namespace feature_detectors;

namespace feature_detectors {

void nonMaxSuppression5x5(std::vector<FeaturePoint>& points, const core::ImageView<core::ImageType::float32>& image) {
    const float* rowPtr1 = image.data + image.stride + 3;
    const float* rowPtr2 = rowPtr1 + image.stride;
    const float* rowPtr3 = rowPtr2 + image.stride;
    const float* rowPtr4 = rowPtr3 + image.stride;
    const float* rowPtr5 = rowPtr4 + image.stride;

    for (size_t row = 3; row < image.rows - 3; row++) {
        for (size_t col = 3; col < image.cols - 3; col++) {
            const float& value = rowPtr3[0];
            if ( // Test the 4 extreme corners first to speed up lazy evaluation
                value > rowPtr1[-2] && value > rowPtr1[2] && value > rowPtr5[-2] && value > rowPtr5[2] &&
                // row 1
                value > rowPtr1[-1] && value > rowPtr1[0] && value > rowPtr1[1] &&
                // row 2
                value > rowPtr2[-2] && value > rowPtr2[-1] && value > rowPtr2[0] && value > rowPtr2[1] && value > rowPtr2[2] &&
                // row 3
                value > rowPtr3[-2] && value > rowPtr3[-1] && value > rowPtr3[1] && value > rowPtr3[2] &&
                // row 4
                value > rowPtr4[-2] && value > rowPtr4[-1] && value > rowPtr4[0] && value > rowPtr4[1] && value > rowPtr4[2] &&
                // row 5
                value > rowPtr5[-1] && value > rowPtr5[0] && value > rowPtr5[1]) {
                FeaturePoint point;
                point.set_x(col);
                point.set_y(row);
                point.set_score(value);
                points.push_back(point);
            }

            rowPtr1++;
            rowPtr2++;
            rowPtr3++;
            rowPtr4++;
            rowPtr5++;
        }

        rowPtr1 += 6;
        rowPtr2 += 6;
        rowPtr3 += 6;
        rowPtr4 += 6;
        rowPtr5 += 6;
    }
}

void computeHarrisStrengths(core::ImageView<core::ImageType::float32>& strengths, core::ImageView<core::ImageType::int16>& Ixx,
    core::ImageView<core::ImageType::int16>& Iyy, core::ImageView<core::ImageType::int16>& Ixy,
    core::ImageView<core::ImageType::int32>& Gxx, core::ImageView<core::ImageType::int32>& Gyy,
    core::ImageView<core::ImageType::int32>& Gxy, const core::ImageView<core::ImageType::uint8>& image, const float k) {

    // dstPtr
    float* dstPtr = strengths.data + strengths.stride * 3 + 3;

    // srcPtr
    const uint8_t* srcPtr1 = image.data;
    const uint8_t* srcPtr2 = srcPtr1 + image.stride;
    const uint8_t* srcPtr3 = srcPtr2 + image.stride;

    // IxxPtr
    int16_t* IxxPtr1 = Ixx.data;
    int16_t* IxxPtr2 = IxxPtr1 + Ixx.stride;
    int16_t* IxxPtr3 = IxxPtr2 + Ixx.stride;
    int16_t* IxxPtr4 = IxxPtr3 + Ixx.stride;
    int16_t* IxxPtr5 = IxxPtr4 + Ixx.stride;

    // IyyPtr
    int16_t* IyyPtr1 = Iyy.data;
    int16_t* IyyPtr2 = IyyPtr1 + Iyy.stride;
    int16_t* IyyPtr3 = IyyPtr2 + Iyy.stride;
    int16_t* IyyPtr4 = IyyPtr3 + Iyy.stride;
    int16_t* IyyPtr5 = IyyPtr4 + Iyy.stride;

    // IxyPtr
    int16_t* IxyPtr1 = Ixy.data;
    int16_t* IxyPtr2 = IxyPtr1 + Ixy.stride;
    int16_t* IxyPtr3 = IxyPtr2 + Ixy.stride;
    int16_t* IxyPtr4 = IxyPtr3 + Ixy.stride;
    int16_t* IxyPtr5 = IxyPtr4 + Ixy.stride;

    // GxxPtr
    int32_t* GxxPtr1 = Gxx.data;
    int32_t* GxxPtr2 = GxxPtr1 + Gxx.stride;
    int32_t* GxxPtr3 = GxxPtr2 + Gxx.stride;
    int32_t* GxxPtr4 = GxxPtr3 + Gxx.stride;
    int32_t* GxxPtr5 = GxxPtr4 + Gxx.stride;

    // GyyPtr
    int32_t* GyyPtr1 = Gyy.data;
    int32_t* GyyPtr2 = GyyPtr1 + Gyy.stride;
    int32_t* GyyPtr3 = GyyPtr2 + Gyy.stride;
    int32_t* GyyPtr4 = GyyPtr3 + Gyy.stride;
    int32_t* GyyPtr5 = GyyPtr4 + Gyy.stride;

    // GxyPtr
    int32_t* GxyPtr1 = Gxy.data;
    int32_t* GxyPtr2 = GxyPtr1 + Gxy.stride;
    int32_t* GxyPtr3 = GxyPtr2 + Gxy.stride;
    int32_t* GxyPtr4 = GxyPtr3 + Gxy.stride;
    int32_t* GxyPtr5 = GxyPtr4 + Gxy.stride;

    details::computeDerivatives(IxxPtr1 + 1, IyyPtr1 + 1, IxyPtr1 + 1, srcPtr1 + 1, srcPtr2 + 1, srcPtr3 + 1, image.cols - 2);
    srcPtr1 += image.stride;
    srcPtr2 += image.stride;
    srcPtr3 += image.stride;
    details::computeDerivatives(IxxPtr2 + 1, IyyPtr2 + 1, IxyPtr2 + 1, srcPtr1 + 1, srcPtr2 + 1, srcPtr3 + 1, image.cols - 2);
    srcPtr1 += image.stride;
    srcPtr2 += image.stride;
    srcPtr3 += image.stride;
    details::computeDerivatives(IxxPtr3 + 1, IyyPtr3 + 1, IxyPtr3 + 1, srcPtr1 + 1, srcPtr2 + 1, srcPtr3 + 1, image.cols - 2);
    srcPtr1 += image.stride;
    srcPtr2 += image.stride;
    srcPtr3 += image.stride;
    details::computeDerivatives(IxxPtr4 + 1, IyyPtr4 + 1, IxyPtr4 + 1, srcPtr1 + 1, srcPtr2 + 1, srcPtr3 + 1, image.cols - 2);
    srcPtr1 += image.stride;
    srcPtr2 += image.stride;
    srcPtr3 += image.stride;

    details::horizontalBinomial(GxxPtr1 + 3, IxxPtr1 + 3, Ixx.cols - 6);
    details::horizontalBinomial(GxxPtr2 + 3, IxxPtr2 + 3, Ixx.cols - 6);
    details::horizontalBinomial(GxxPtr3 + 3, IxxPtr3 + 3, Ixx.cols - 6);
    details::horizontalBinomial(GxxPtr4 + 3, IxxPtr4 + 3, Ixx.cols - 6);

    details::horizontalBinomial(GyyPtr1 + 3, IyyPtr1 + 3, Iyy.cols - 6);
    details::horizontalBinomial(GyyPtr2 + 3, IyyPtr2 + 3, Iyy.cols - 6);
    details::horizontalBinomial(GyyPtr3 + 3, IyyPtr3 + 3, Iyy.cols - 6);
    details::horizontalBinomial(GyyPtr4 + 3, IyyPtr4 + 3, Iyy.cols - 6);

    details::horizontalBinomial(GxyPtr1 + 3, IxyPtr1 + 3, Ixy.cols - 6);
    details::horizontalBinomial(GxyPtr2 + 3, IxyPtr2 + 3, Ixy.cols - 6);
    details::horizontalBinomial(GxyPtr3 + 3, IxyPtr3 + 3, Ixy.cols - 6);
    details::horizontalBinomial(GxyPtr4 + 3, IxyPtr4 + 3, Ixy.cols - 6);

    for (size_t row = 4; row < image.rows - 2; row++) {
        details::computeDerivatives(IxxPtr5 + 1, IyyPtr5 + 1, IxyPtr5 + 1, srcPtr1 + 1, srcPtr2 + 1, srcPtr3 + 1, image.cols - 2);
        srcPtr1 += image.stride;
        srcPtr2 += image.stride;
        srcPtr3 += image.stride;

        details::horizontalBinomial(GxxPtr5 + 3, IxxPtr5 + 3, Ixx.cols - 6);
        details::horizontalBinomial(GyyPtr5 + 3, IyyPtr5 + 3, Iyy.cols - 6);
        details::horizontalBinomial(GxyPtr5 + 3, IxyPtr5 + 3, Ixy.cols - 6);

        details::verticalBinomial(GxxPtr1 + 3, GxxPtr1 + 3, GxxPtr2 + 3, GxxPtr3 + 3, GxxPtr4 + 3, GxxPtr5 + 3, Ixx.cols - 6);
        details::verticalBinomial(GyyPtr1 + 3, GyyPtr1 + 3, GyyPtr2 + 3, GyyPtr3 + 3, GyyPtr4 + 3, GyyPtr5 + 3, Iyy.cols - 6);
        details::verticalBinomial(GxyPtr1 + 3, GxyPtr1 + 3, GxyPtr2 + 3, GxyPtr3 + 3, GxyPtr4 + 3, GxyPtr5 + 3, Ixy.cols - 6);

        details::harrisStengths(dstPtr, GxxPtr1 + 3, GyyPtr1 + 3, GxyPtr1 + 3, image.cols - 6, k);

        dstPtr += strengths.stride;

        details::swapPointers(&IxxPtr1, &IxxPtr2, &IxxPtr3, &IxxPtr4, &IxxPtr5);
        details::swapPointers(&IyyPtr1, &IyyPtr2, &IyyPtr3, &IyyPtr4, &IyyPtr5);
        details::swapPointers(&IxyPtr1, &IxyPtr2, &IxyPtr3, &IxyPtr4, &IxyPtr5);
        details::swapPointers(&GxxPtr1, &GxxPtr2, &GxxPtr3, &GxxPtr4, &GxxPtr5);
        details::swapPointers(&GyyPtr1, &GyyPtr2, &GyyPtr3, &GyyPtr4, &GyyPtr5);
        details::swapPointers(&GxyPtr1, &GxyPtr2, &GxyPtr3, &GxyPtr4, &GxyPtr5);
    }
}
}

const std::vector<FeaturePoint>& HarrisFeatureDetector::detect(const core::ImageView<core::ImageType::uint8>& image) {
    if (image.rows != m_strengths.rows() || image.cols != m_strengths.cols()) {
        m_Ixx = core::DynamicImage<core::ImageType::int16, int16_aligned_storage_t>(5, image.cols, image.stride);
        m_Iyy = core::DynamicImage<core::ImageType::int16, int16_aligned_storage_t>(5, image.cols, image.stride);
        m_Ixy = core::DynamicImage<core::ImageType::int16, int16_aligned_storage_t>(5, image.cols, image.stride);
        m_Gxx = core::DynamicImage<core::ImageType::int32, int32_aligned_storage_t>(5, image.cols, image.stride);
        m_Gyy = core::DynamicImage<core::ImageType::int32, int32_aligned_storage_t>(5, image.cols, image.stride);
        m_Gxy = core::DynamicImage<core::ImageType::int32, int32_aligned_storage_t>(5, image.cols, image.stride);
        m_strengths = core::DynamicImage<core::ImageType::float32, float32_aligned_storage_t>(image.rows, image.cols, image.stride);
    }

    m_points.clear();

    auto IxxView = m_Ixx.view();
    auto IyyView = m_Iyy.view();
    auto IxyView = m_Ixy.view();
    auto GxxView = m_Gxx.view();
    auto GyyView = m_Gyy.view();
    auto GxyView = m_Gxy.view();
    auto strengthView = m_strengths.view();

    computeHarrisStrengths(strengthView, IxxView, IyyView, IxyView, GxxView, GyyView, GxyView, image, m_k);
    nonMaxSuppression5x5(m_points, strengthView);

    return m_points;
}