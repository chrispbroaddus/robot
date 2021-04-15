#include "glog/logging.h"
#include "packages/benchmarking/include/benchmark.h"
#include "packages/dense_mapping/include/box_geometry.h"
#include "packages/logging/include/syslog_sink.h"
#include <atomic>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>

namespace {
constexpr double minimumX = -100;
constexpr double maximumX = 100;
constexpr double minimumY = -100;
constexpr double maximumY = 100;
constexpr double minimumZ = -100;
constexpr double maximumZ = 100;
constexpr int boxMinimumX = -49;
constexpr int boxMaximumX = 50;
constexpr int boxMinimumY = -49;
constexpr int boxMaximumY = 50;
constexpr int boxMinimumZ = -49;
constexpr int boxMaximumZ = 50;
constexpr double boxCenterOffset = -0.5;
constexpr double boxHalfExtent = 1;

std::uniform_real_distribution<double> xDirection(minimumX, maximumX);
std::uniform_real_distribution<double> yDirection(minimumY, maximumY);
std::uniform_real_distribution<double> zDirection(minimumZ, maximumZ);

// Ensure we have good random numbers when performing this test. Hopefully what this means is that each thread gets
// its own, unique seed.
std::atomic<std::mt19937::result_type> counter;

// Give each thread its own PRNG that is cached
thread_local std::mt19937 prng(++counter);

// Need to force the things we are benchmarking to cause side effects so that they are not optimized away. In this case,
// we just count how many intersections we find and return that result. This gets ignored later on.
int baseline() {
    int count = 0;
    std::array<double, 3> sourcePoint{ { xDirection(prng), yDirection(prng), zDirection(prng) } };
    std::array<double, 3> targetPoint{ { xDirection(prng), yDirection(prng), zDirection(prng) } };

    for (int x = boxMinimumX; x <= boxMaximumX; ++x) {
        for (int y = boxMinimumY; y <= boxMaximumY; ++y) {
            for (int z = boxMinimumZ; z <= boxMaximumZ; ++z) {
                std::array<double, 3> boxCenter{ { x + boxCenterOffset, y + boxCenterOffset, z + boxCenterOffset } };
                count
                    += dense_mapping::Geometry::segmentIntersectsAxisAlignedBoundingBox(sourcePoint, targetPoint, boxCenter, boxHalfExtent)
                    ? 1
                    : 0;
            }
        }
    }
    return count;
}

// Same as above -- need to cause side effects linked to the things we're timing to ensure that they don't get optimized
// away.
int comparison() {
    int count = 0;

    std::array<double, 3> sourcePoint{ { xDirection(prng), yDirection(prng), zDirection(prng) } };
    std::array<double, 3> targetPoint{ { xDirection(prng), yDirection(prng), zDirection(prng) } };
    std::array<double, 3> direction = dense_mapping::Geometry::directionVector(sourcePoint, targetPoint);
    std::array<double, 3> reciprocalDirection = dense_mapping::Geometry::reciprocalDirectionVector(direction);

    for (int x = boxMinimumX; x <= boxMaximumX; ++x) {
        for (int y = boxMinimumY; y <= boxMaximumY; ++y) {
            for (int z = boxMinimumZ; z <= boxMaximumZ; ++z) {
                std::array<double, 3> boxCenter{ { x + boxCenterOffset, y + boxCenterOffset, z + boxCenterOffset } };
                count += dense_mapping::Geometry::segmentIntersectsAxisAlignedBoundingBox(
                             boxHalfExtent, boxCenter, sourcePoint, reciprocalDirection)
                    ? 1
                    : 0;
            }
        }
    }

    return count;
}
}

int main(int, char**) {
    constexpr size_t innerIterations = 250;
    const size_t outerIterations = std::thread::hardware_concurrency() * 1000;
    const auto result = performBenchmark(&baseline, &comparison, outerIterations, innerIterations);
    const auto maximum = std::max(
        result.baselineOuterLoopStatisticsSecondsPerIteration.mean(), result.comparisonOuterLoopStatisticsSecondsPerIteration.mean());
    const auto percentChangeLower = 100 * result.analysis.differenceConfidenceIntervalLowerLimit / maximum;
    const auto percentChangeUpper = 100 * result.analysis.differenceConfidenceIntervalUpperLimit / maximum;

    logging::useSyslogSink();

    LOG(INFO) << std::fixed << std::setprecision(15) << "Statistics:" << std::endl
              << "  Baseline Implementation:" << std::endl
              << "    Count:                 " << std::setw(20) << std::right
              << result.baselineOuterLoopStatisticsSecondsPerIteration.count() << std::endl
              << "    Minimum (s):           " << std::setw(20) << std::right
              << result.baselineOuterLoopStatisticsSecondsPerIteration.minimum() << std::endl
              << "    Maximum (s):           " << std::setw(20) << std::right
              << result.baselineOuterLoopStatisticsSecondsPerIteration.maximum() << std::endl
              << "    Mean (s):              " << std::setw(20) << std::right
              << result.baselineOuterLoopStatisticsSecondsPerIteration.mean() << std::endl
              << "    St. Dev.:              " << std::setw(20) << std::right
              << result.baselineOuterLoopStatisticsSecondsPerIteration.standardDeviation() << std::endl
              << "  Comparison Implementation:" << std::endl
              << "    Count:                 " << std::setw(20) << std::right
              << result.comparisonOuterLoopStatisticsSecondsPerIteration.count() << std::endl
              << "    Minimum (s):           " << std::setw(20) << std::right
              << result.comparisonOuterLoopStatisticsSecondsPerIteration.minimum() << std::endl
              << "    Maximum (s):           " << std::setw(20) << std::right
              << result.comparisonOuterLoopStatisticsSecondsPerIteration.maximum() << std::endl
              << "    Mean (s):              " << std::setw(20) << std::right
              << result.comparisonOuterLoopStatisticsSecondsPerIteration.mean() << std::endl
              << "    St. Dev.:              " << std::setw(20)
              << result.comparisonOuterLoopStatisticsSecondsPerIteration.standardDeviation() << std::endl
              << "  Analysis:" << std::endl
              << "    T-statistic:           " << std::setw(40) << std::right << result.analysis.tStatistic << std::endl
              << "    Effective D.o.F.:      " << std::setw(40) << std::right << result.analysis.effectiveDegreesOfFreedom << std::endl
              << "    Pr[|t| > T]: by chance " << std::setw(40) << std::right << 100 * result.analysis.probabilityMeansEqual << "%"
              << std::endl
              << "    Difference 95% CI LL:  " << std::setw(40) << std::right << result.analysis.differenceConfidenceIntervalLowerLimit
              << std::endl
              << "    Difference 95% CI UL:  " << std::setw(40) << std::right << result.analysis.differenceConfidenceIntervalUpperLimit
              << std::endl
              << "  Likely % change:         [" << std::setw(40) << std::right << percentChangeLower << ", " << std::setw(40) << std::right
              << percentChangeUpper << "] %" << std::endl;

    return 0;
}
