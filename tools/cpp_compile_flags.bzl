WARNING_FLAGS = [
    "-Wunused",
    "-Wall",
    "-Wextra",
    "-Werror",
    "-Wnoexcept",
    "-Wdouble-promotion",
    "-Wformat=2",
    "-Wuninitialized",
    "-Wmaybe-uninitialized",
    "-Wstrict-aliasing",
    "-Warray-bounds=2",
    "-Wlogical-op",
    "-pedantic",
    "-pedantic-errors",
]

LANGUAGE_FLAGS = [
    "-std=c++14",
    "-fstrict-aliasing",
    "-ftree-vrp",
]

# Prefer NaN initialization for Eigen, such that uninitizalized eigen members
# are immediately identifiable
EIGEN_COPTS = [
    "-DEIGEN_INITIALIZE_MATRICES_BY_NAN",
]


COPTS = WARNING_FLAGS + LANGUAGE_FLAGS + EIGEN_COPTS