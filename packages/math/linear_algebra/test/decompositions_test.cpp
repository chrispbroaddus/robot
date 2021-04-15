#include "packages/math/linear_algebra/decompositions.h"

#include "gtest/gtest.h"

TEST(DecompositionsTest, testGivensRotation) {

    Eigen::Matrix<double, -1, -1> A(3, 3);
    A << 2, 1, 5, 3, 8, 2, 4, 6, 7;
    Eigen::Matrix<double, 3, 3> ACopy = A;
    Eigen::Matrix<double, -1, -1> Qt(3, 3);
    Qt = Eigen::Matrix<double, 3, 3>::Identity(3, 3);

    double c, s, r;
    linear_algebra::computeGivensRotation<double>(A(1, 0), A(2, 0), c, s, r);
    linear_algebra::applyGivensRotationToMatrix<double>(A, c, s, 1, 2);
    linear_algebra::applyGivensRotationToMatrix<double>(Qt, c, s, 1, 2);

    linear_algebra::computeGivensRotation<double>(A(0, 0), A(1, 0), c, s, r);
    linear_algebra::applyGivensRotationToMatrix<double>(A, c, s, 0, 1);
    linear_algebra::applyGivensRotationToMatrix<double>(Qt, c, s, 0, 1);

    linear_algebra::computeGivensRotation<double>(A(1, 1), A(2, 1), c, s, r);
    linear_algebra::applyGivensRotationToMatrix<double>(A, c, s, 1, 2);
    linear_algebra::applyGivensRotationToMatrix<double>(Qt, c, s, 1, 2);

    Eigen::Matrix<double, 3, 3> ARecovered = Qt.transpose() * A;
    EXPECT_NEAR(0, A(1, 0), 1.0e-6);
    EXPECT_NEAR(0, A(2, 0), 1.0e-6);
    EXPECT_NEAR(0, A(2, 1), 1.0e-6);

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            EXPECT_NEAR(ACopy(i, j), ARecovered(i, j), 1.0e-6);
        }
    }
}

TEST(DecompositionsTest, testQRDecompositionEigenSmallMatrix) {

    std::srand(1234);
    Eigen::Matrix<double, -1, -1> A(4, 3);
    A = Eigen::Matrix<double, 4, 3>::Random(4, 3);
    Eigen::Matrix<double, -1, -1> Q;
    Eigen::Matrix<double, -1, -1> R;

    linear_algebra::qrDecompositionEigen(A, Q, R);
}

TEST(DecompositionsTest, testQRDecompositionEigenLargeMatrix) {

    std::srand(1234);
    Eigen::Matrix<double, -1, -1> A(2000, 81);
    A = Eigen::Matrix<double, 2000, 81>::Random(2000, 81);
    Eigen::Matrix<double, -1, -1> Q;
    Eigen::Matrix<double, -1, -1> R;

    linear_algebra::qrDecompositionEigen(A, Q, R);
}

TEST(DecompositionsTest, testQRDecomposition) {

    std::srand(1234);
    Eigen::Matrix<double, -1, -1> Q(4, 4);
    Q << 0.2480694691784169, -0.2785097551486035, 0.6039854097051024, 0.7043404571739176, 0.3721042037676254, 0.8208708572800949,
        -0.2137701169180982, 0.3768445758127966, 0.4961389383568338, 0.2052177143200237, 0.5918972185698528, -0.6011568233204139,
        0.7442084075352507, -0.4544106531371954, -0.4890415571558869, -0.0224312247507616;
    Eigen::Matrix<double, -1, -1> R(4, 3);
    R << 8.0622577482985491, 9.9227787671366769, 7.6901535445309239, 0.0000000000000000, 5.2477101233263195, 0.3224849796457518,
        0.0000000000000000, 0.0000000000000000, 5.2685426732106224, 0.0000000000000000, 0.0000000000000000, 0.0000000000000000;
    Eigen::Matrix<double, -1, -1> A(4, 3);
    A = Q * R;
    Eigen::Matrix<double, -1, -1> X(3, 1);
    X = Eigen::Matrix<double, 3, 1>::Random(3, 1);
    Eigen::Matrix<double, -1, 1> y(4, 1);
    y = A * X;
    Eigen::Matrix<double, -1, 1> QTy(4, 1);
    QTy = Q.transpose() * y;

    linear_algebra::applyQRBothSides(A, y);

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            EXPECT_NEAR(R(i, j), A(i, j), 2.0e-15);
        }
    }
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 1; j++) {
            EXPECT_NEAR(QTy(i, j), y(i, j), 2.0e-15);
        }
    }
}

TEST(DecompositionsTest, UpperTriangularRLargeMatrixTest) {

    std::srand(1234);
    Eigen::Matrix<double, -1, -1> A(2000, 81);
    A = Eigen::Matrix<double, 2000, 81>::Random(2000, 81);
    Eigen::Matrix<double, -1, 1> y(2000, 1);
    y = Eigen::Matrix<double, 2000, 1>::Random(2000, 1);

    linear_algebra::applyQRBothSides(A, y);

    for (int i = 0; i < 81; i++) {
        for (int j = 0; j < 81; j++) {
            if (i > j) {
                EXPECT_NEAR(0, A(i, j), 1.0e-15);
            }
        }
    }
}
