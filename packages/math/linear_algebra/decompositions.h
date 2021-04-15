#pragma once

#include "Eigen/Eigen"
#include <stdexcept>

namespace linear_algebra {

/// \details Compute Givens Rotation that takes a vector (a, b) to (r, 0)
/// | c -s | | a | = | r |
/// | s  c | | b |   | 0 |
/// r > 0 is guaranteed
template <typename T> inline void computeGivensRotation(const T& a, const T& b, T& c, T& s, T& r) {
    r = std::sqrt(a * a + b * b);
    T oneOverR = (T)1.0 / r;
    c = a * oneOverR;
    s = -b * oneOverR;
}

/// \details Apply Given's rotation to a Matrix
/// \tparam T - data-type of the matrix
/// \param matrix - Matrix to be transformed
/// \param c - cosine of the rotation angle
/// \param s - sine of the rotation angle
/// \param row1 - top row number to apply the Givens rotation on
/// \param row2 - bottom row number to apply the Givens rotation on
/// \param buffer - Temporary buffer to store intermediate values (Must have the same number of columns as matrix)
template <typename T>
inline void applyGivensRotationToMatrix(Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& matrix, T c, T s, uint32_t row1, uint32_t row2) {

    T row1Val;
    T row2Val;
    for (int col = 0; col < matrix.cols(); col++) {
        row1Val = matrix.coeffRef(row1, col);
        row2Val = matrix.coeffRef(row2, col);
        matrix.coeffRef(row1, col) = c * row1Val - s * row2Val;
        matrix.coeffRef(row2, col) = s * row1Val + c * row2Val;
    }
}

/// \details Apply Given's rotation to a vector
/// \tparam T - data-type of the matrix
/// \param vector - vector to be transformed
/// \param c - cosine of the rotation angle
/// \param s - sine of the rotation angle
/// \param row1 - top row number to apply the Givens rotation on
/// \param row2 - bottom row number to apply the Givens rotation on
template <typename T>
inline void applyGivensRotationToVector(Eigen::Matrix<T, Eigen::Dynamic, 1>& vector, T c, T s, uint32_t row1, uint32_t row2) {

    T row1Val = vector.coeffRef(row1, 0);
    T row2Val = vector.coeffRef(row2, 0);
    vector.coeffRef(row1, 0) = c * row1Val - s * row2Val;
    vector.coeffRef(row2, 0) = s * row1Val + c * row2Val;
}

/// \details Destructive apply Q^T on a matrix and a vector where Q is an orthogonal factor of A
/// \tparam T matrix type
/// \param A Matrix to decompose - It will be destroyed and replaced by R (upper triangular)
/// \param y - It will be destroyed and replaced by Q^T * y
template <typename T> void applyQRBothSides(Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& A, Eigen::Matrix<T, Eigen::Dynamic, 1>& y) {

    if (y.rows() != A.rows()) {
        throw std::runtime_error("Mismatch in the number of rows in A and y");
    }

    const long m = A.rows();
    const long n = A.cols();

    for (uint32_t col = 0; col < n; col++) {
        for (uint32_t row = (uint32_t)m - 1; row > col; row--) {
            if (A.coeffRef(row, col) == 0) {
                continue;
            }
            T c, s, r;
            linear_algebra::computeGivensRotation<T>(A.coeffRef(row - 1, col), A.coeffRef(row, col), c, s, r);
            linear_algebra::applyGivensRotationToMatrix<T>(A, c, s, row - 1, row);
            linear_algebra::applyGivensRotationToVector<T>(y, c, s, row - 1, row);
        }
    }
    A = A.block(0, 0, n, n).eval();
    y = y.block(0, 0, n, 1).eval();
}

/// \details QR decomposition using Eigen A = Q * R
/// \tparam T - Matrix data-type
/// \param A - Matrix to decompose
/// \param Q - Orthogonal Q matrix
/// \param R - Upper-triangular R matrix
template <typename T>
void qrDecompositionEigen(Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& A, Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& Q,
    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& R) {

    Eigen::HouseholderQR<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> > householderQR(A);
    Q = householderQR.householderQ();
    householderQR.compute(A);
    R = householderQR.matrixQR().template triangularView<Eigen::Upper>();
}
}
