cc_library(
    name = "eigen",
    hdrs = glob([
        "Eigen/src/Cholesky/*.h",
        "Eigen/src/Core/arch/**/*.h",
        "Eigen/src/Core/functors/*.h",
        "Eigen/src/Core/products/*.h",
        "Eigen/src/Core/util/*.h",
        "Eigen/src/Core/*.h",
        "Eigen/src/Eigenvalues/*.h",
        "Eigen/src/Geometry/*.h",
        "Eigen/src/Geometry/arch/**/*.h",
        "Eigen/src/Householder/*.h",
        "Eigen/src/Jacobi/*.h",
        "Eigen/src/LU/*.h",
        "Eigen/src/LU/arch/*.h",
        "Eigen/src/plugins/*.h",
        "Eigen/src/SVD/*.h",
        "Eigen/src/QR/*.h",
        "Eigen/src/misc/*.h",
        "Eigen/SparseCholesky/*.h",
    ]) + 
    glob(["Eigen/src/StlSupport/**/*.h"]) + 
    glob(["Eigen/src/SparseCore/**/*.h"]) + 
    glob(["Eigen/src/OrderingMethods/**/*.h"]) + 
    glob(["Eigen/src/SparseCholesky/**/*.h"]) + 
    glob(["Eigen/src/SparseLU/**/*.h"]) + 
    glob(["Eigen/src/SparseQR/**/*.h"]) + 
    glob(["Eigen/src/IterativeLinearSolvers/**/*.h"]) + 
    [
        "Eigen/Eigen",
        "Eigen/Core",
        "Eigen/Dense",
        "Eigen/Sparse",
        "Eigen/SparseLU",
        "Eigen/SparseQR",
        "Eigen/SparseCholesky",
        "Eigen/LU",
        "Eigen/Cholesky",
        "Eigen/Eigenvalues",
        "Eigen/Householder",
        "Eigen/OrderingMethods",
        "Eigen/Geometry",
        "Eigen/Jacobi",
        "Eigen/SparseCore",
        "Eigen/QR",
        "Eigen/SVD",
        "Eigen/StdDeque",
        "Eigen/StdList",
        "Eigen/StdVector",
        "Eigen/IterativeLinearSolvers",
        "unsupported/Eigen/SpecialFunctions",
        "unsupported/Eigen/CXX11/ThreadPool",
        "unsupported/Eigen/CXX11/Tensor",
    ],
    includes = ["Eigen/", "Eigen/src"],
    visibility = ["//visibility:public"],
)
