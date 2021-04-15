#pragma once

#include "packages/calibration/include/kannala_brandt_distortion_model.h"
#include "packages/calibration/include/linear_camera_model.h"
#include "packages/hal/proto/camera_sample.pb.h"

#include "Eigen"

namespace calibration {
template <typename T> class Kb4ImageUndistortion {
public:
    /// Create a look-up table for mapping undistorted pixels to distorted pixels
    /// \param K Camera Matrix of the camera that captured the distorted image
    /// \param coef Distortion coefficients of camera matrix
    /// \param outputRows Number of rows in the output(undistorted) image
    /// \param outputCols Number of cols in the output(undistorted) image
    /// \param newK Camera Matrix that defines the describes the undistorted image
    Kb4ImageUndistortion(const Eigen::Matrix<T, 3, 3>& K, const Eigen::Matrix<T, 4, 1>& coef, uint32_t outputRows, uint32_t outputCols,
        const Eigen::Matrix<T, 3, 3>& newK)
        : m_kb4Model(K, coef, 10, 0)
        , m_linearModel(newK)
        , m_outputRows(outputRows)
        , m_outputCols(outputCols) {

        m_xCoordinates.resize(m_outputRows * m_outputCols);
        m_yCoordinates.resize(m_outputRows * m_outputCols);
        int index;
        Eigen::Matrix<T, 3, 1> ray;
        Eigen::Matrix<T, 2, 1> inputPixel;
        Eigen::Matrix<T, 2, 1> outputPixel;
        for (uint32_t rowCtr = 0; rowCtr < m_outputRows; rowCtr++) {
            for (uint32_t colCtr = 0; colCtr < m_outputCols; colCtr++) {
                outputPixel(0) = colCtr;
                outputPixel(1) = rowCtr;
                ray = m_linearModel.unproject(outputPixel);
                index = rowCtr * m_outputCols + colCtr;
                inputPixel = m_kb4Model.project(ray);
                m_xCoordinates[index] = inputPixel(0);
                m_yCoordinates[index] = inputPixel(1);
            }
        }
    }

    /// Undistort an input image using the
    /// \param distortedImage
    /// \param undistortedImage
    void undistortImage(const hal::Image& distortedImage, hal::Image& undistortedImage) {

        undistortedImage.mutable_info()->CopyFrom(distortedImage.info());

        uint32_t bytesPerPixel;
        if (distortedImage.type() == hal::PB_BYTE || distortedImage.type() == hal::PB_UNSIGNED_BYTE) {
            bytesPerPixel = 1;
            undistortedImage.set_type(hal::PB_UNSIGNED_BYTE);
        } else {
            throw std::runtime_error("Unsupported image type: " + std::to_string(distortedImage.type()));
        }

        if (distortedImage.format() == hal::PB_LUMINANCE || distortedImage.format() == hal::PB_RAW) {
            bytesPerPixel *= 1;
            undistortedImage.set_format(hal::PB_LUMINANCE);
        } else if (distortedImage.format() == hal::PB_RGB) {
            bytesPerPixel *= 3;
            undistortedImage.set_format(hal::PB_RGB);
        } else {
            throw std::runtime_error("Unsupported image format: " + std::to_string(distortedImage.format()));
        }
        undistortedImage.set_cols(m_outputCols);
        undistortedImage.set_rows(m_outputRows);
        undistortedImage.set_stride(m_outputCols * bytesPerPixel);

        std::vector<unsigned char> imageData;
        imageData.resize(m_outputRows * m_outputCols * bytesPerPixel);
        const unsigned char* inputPtr = (const unsigned char*)(distortedImage.data().data());
        uint32_t inputRows = distortedImage.rows();
        uint32_t inputCols = distortedImage.cols();

        for (uint32_t rowCtr = 0; rowCtr < m_outputRows; rowCtr++) {
            for (uint32_t colCtr = 0; colCtr < m_outputCols; colCtr++) {
                int inputRow = (int)std::round(m_yCoordinates[rowCtr * m_outputCols + colCtr]);
                int inputCol = (int)std::round(m_xCoordinates[rowCtr * m_outputCols + colCtr]);
                if (inputRow < 0 || inputCol < 0 || inputRow >= (int)inputRows || inputCol >= (int)inputCols) {
                    continue;
                }
                if (bytesPerPixel == 1) {
                    imageData[rowCtr * m_outputCols + colCtr] = inputPtr[inputRow * inputCols + inputCol];
                } else if (bytesPerPixel == 3) {
                    imageData[(rowCtr * m_outputCols + colCtr) * 3] = inputPtr[(inputRow * inputCols + inputCol) * 3];
                    imageData[(rowCtr * m_outputCols + colCtr) * 3 + 1] = inputPtr[(inputRow * inputCols + inputCol) * 3 + 1];
                    imageData[(rowCtr * m_outputCols + colCtr) * 3 + 2] = inputPtr[(inputRow * inputCols + inputCol) * 3 + 2];
                }
            }
        }
        undistortedImage.set_data(imageData.data(), imageData.size());
    }

private:
    KannalaBrandtRadialDistortionModel4<T> m_kb4Model;
    LinearCameraModel<T> m_linearModel;
    const uint32_t m_outputRows;
    const uint32_t m_outputCols;
    std::vector<float> m_xCoordinates;
    std::vector<float> m_yCoordinates;
};
}
