#include "../../benchmarking/include/summary_statistics.h"
#include "../include/insert_policies.h"
#include "../include/octree_payloads.h"
#include "../include/octree_queries.h"
#include "glog/logging.h"
#include "packages/benchmarking/include/benchmark.h"
#include "packages/dense_mapping/include/box_geometry.h"
#include "packages/dense_mapping/include/insert_policies.h"
#include "packages/dense_mapping/include/octree.h"
#include "packages/dense_mapping/include/octree_payloads.h"
#include "packages/dense_mapping/include/octree_queries.h"
#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <thread>

namespace dense_mapping {
namespace Geometry {
    /// Determine whether the line segment defined by (source, source + direction) (@emph{note the scaling!}) has any
    /// intersection with the box with given center and half-extent (half height/width/depth).
    ///
    /// Based on https://tavianator.com/fast-branchless-raybounding-box-intersections/ and
    /// http://people.csail.mit.edu/amy/papers/box-jgt.pdf -- hypothetically, this should be compilable into branchless
    /// assembly code, which is potentially significantly faster because there are no pipeline stalls. This hasn't
    /// been benchmarked yet, so we can't verify those claims yet.
    ///
    /// This correctly handles a number of special cases:
    /// -# Degenerate segments (those where source = destination and therefore direction = 0) -- these produce results
    ///    which are identical to simply performing a point/box intersection test.
    /// -# Segments which start or end on a corner, edge, or face of the box (up to machine precision)
    /// .
    ///
    /// \tparam T Floating point type
    /// \param source one end point of the segment
    /// \param direction Exactly (destination - source)
    /// \param boxCenter
    /// \param boxHalfExtent
    /// \return An object which is implicitly convertible to bool. Right now, this object also contains additional
    /// book-keeping information to assist in debugging and is also a hedge to enable additional use cases (e.g.
    /// generalizing to general ray intersection or line intersection tests, which all rely on the same calculations
    /// but which use slightly different logic to interpret results).
    template <typename T>
    constexpr SegmentAxisAlignedBoundingBoxIntersection<T> segmentIntersectsAxisAlignedBoundingBox2(
        std::array<T, 3> source, std::array<T, 3> direction, std::array<T, 3> boxCenter, T boxHalfExtent) {
        const auto boxMinX = boxCenter[0] - boxHalfExtent;
        const auto boxMinY = boxCenter[1] - boxHalfExtent;
        const auto boxMinZ = boxCenter[2] - boxHalfExtent;
        const auto boxMaxX = boxCenter[0] + boxHalfExtent;
        const auto boxMaxY = boxCenter[1] + boxHalfExtent;
        const auto boxMaxZ = boxCenter[2] + boxHalfExtent;

        // Compute the unscaled distance to the closest / farthest plane. Here we take advantage of our knowledge of the
        // sign of the reciprocal direction to avoid additional min/max calls (see the Utah paper for details as to why)
        const auto dMinX = (direction[0] >= 0 ? (boxMinX - source[0]) : (boxMaxX - source[0]));
        const auto dMaxX = (direction[0] >= 0 ? (boxMaxX - source[0]) : (boxMinX - source[0]));
        const auto dMinY = (direction[1] >= 0 ? (boxMinY - source[1]) : (boxMaxY - source[1]));
        const auto dMaxY = (direction[1] >= 0 ? (boxMaxY - source[1]) : (boxMinY - source[1]));
        const auto dMinZ = (direction[2] >= 0 ? (boxMinZ - source[2]) : (boxMaxZ - source[2]));
        const auto dMaxZ = (direction[2] >= 0 ? (boxMaxZ - source[2]) : (boxMinZ - source[2]));

        // This attempts to be clever: in the case that the source or target point is not exactly on the face of the box,
        // this is exactly as you would expect if you've read about any of the slab testing approaches for AABB. On the
        // other hand, if we are exactly on one of those planes and we do not move parallel to that plane, the reciprocal
        // direction will be +/-inf, which will potentially cause NaNs in the computed tMin/tMax. To solve this,
        // we declare that if we are on a minimum face, we have a distance of exactly -infinity, otherwise if we are on a
        // maximum face we are at a distance of exactly infinity. This seems to appease the unit test gods and is kinda sensible.
        const auto tMinX = dMinX != 0 ? dMinX / direction[0] : -std::numeric_limits<T>::infinity();
        const auto tMinY = dMinY != 0 ? dMinY / direction[1] : -std::numeric_limits<T>::infinity();
        const auto tMinZ = dMinZ != 0 ? dMinZ / direction[2] : -std::numeric_limits<T>::infinity();
        const auto tMaxX = dMaxX != 0 ? dMaxX / direction[0] : std::numeric_limits<T>::infinity();
        const auto tMaxY = dMaxY != 0 ? dMaxY / direction[1] : std::numeric_limits<T>::infinity();
        const auto tMaxZ = dMaxZ != 0 ? dMaxZ / direction[2] : std::numeric_limits<T>::infinity();

        const auto tMin = std::max(tMinX, std::max(tMinY, tMinZ));
        const auto tMax = std::min(tMaxX, std::min(tMaxY, tMaxZ));

        return SegmentAxisAlignedBoundingBoxIntersection<T>(tMin, tMax);
    }
}
}

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

std::atomic<std::mt19937::result_type> counter;
thread_local std::mt19937 prng(++counter);

// From https://tavianator.com/fast-branchless-raybounding-box-intersections-part-2-nans/
template <typename T>
bool intersectionTavianator(std::array<T, 3> origin, std::array<T, 3> reciprocalDirection, std::array<T, 3> boxCenter, T boxHalfExtent) {
    auto t1 = ((boxCenter[0] - boxHalfExtent) - origin[0]) * reciprocalDirection[0];
    auto t2 = ((boxCenter[0] + boxHalfExtent) - origin[0]) * reciprocalDirection[0];

    auto tmin = std::min(t1, t2);
    auto tmax = std::max(t1, t2);

    for (int i = 1; i < 3; ++i) {
        t1 = ((boxCenter[i] - boxHalfExtent) - origin[i]) * reciprocalDirection[i];
        t2 = ((boxCenter[i] + boxHalfExtent) - origin[i]) * reciprocalDirection[i];

        tmin = std::max(tmin, std::min(std::min(t1, t2), tmax));
        tmax = std::min(tmax, std::max(std::max(t1, t2), tmin));
    }

    return (tmax >= tmin) && (tmax >= 0) && (tmin <= 1);
}

int baselineBoxSegment() {
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

int boxSegmentIntersectionTavianator() {
    int count = 0;

    std::array<double, 3> sourcePoint{ { xDirection(prng), yDirection(prng), zDirection(prng) } };
    std::array<double, 3> targetPoint{ { xDirection(prng), yDirection(prng), zDirection(prng) } };
    std::array<double, 3> direction = dense_mapping::Geometry::directionVector(sourcePoint, targetPoint);
    std::array<double, 3> reciprocalDirection = dense_mapping::Geometry::reciprocalDirectionVector(direction);

    for (int x = boxMinimumX; x <= boxMaximumX; ++x) {
        for (int y = boxMinimumY; y <= boxMaximumY; ++y) {
            for (int z = boxMinimumZ; z <= boxMaximumZ; ++z) {
                std::array<double, 3> boxCenter{ { x + boxCenterOffset, y + boxCenterOffset, z + boxCenterOffset } };
                count += intersectionTavianator(sourcePoint, reciprocalDirection, boxCenter, boxHalfExtent) ? 1 : 0;
            }
        }
    }

    return count;
}

int boxSegmentIntersectionNoReciprocalMultiply() {
    int count = 0;

    std::array<double, 3> sourcePoint{ { xDirection(prng), yDirection(prng), zDirection(prng) } };
    std::array<double, 3> targetPoint{ { xDirection(prng), yDirection(prng), zDirection(prng) } };
    std::array<double, 3> direction = dense_mapping::Geometry::directionVector(sourcePoint, targetPoint);

    for (int x = boxMinimumX; x <= boxMaximumX; ++x) {
        for (int y = boxMinimumY; y <= boxMaximumY; ++y) {
            for (int z = boxMinimumZ; z <= boxMaximumZ; ++z) {
                std::array<double, 3> boxCenter{ { x + boxCenterOffset, y + boxCenterOffset, z + boxCenterOffset } };
                count += dense_mapping::Geometry::segmentIntersectsAxisAlignedBoundingBox2(sourcePoint, direction, boxCenter, boxHalfExtent)
                    ? 1
                    : 0;
            }
        }
    }

    return count;
}

template <typename T> void logResults(const SummaryStatistics<T>& summary) {
    const auto maximumThroughput = 1 / summary.minimum();
    const auto minimumThroughput = 1 / summary.maximum();
    const auto averageThroughput = 1 / summary.mean();

    LOG(INFO) << std::fixed << std::setprecision(15) << std::setfill(' ') << "Timing Statistics:" << std::endl
              << "  Count:                 " << summary.count() << std::endl
              << "  Minimum (s):           " << std::setw(20) << summary.minimum() << std::endl
              << "  Maximum (s):           " << std::setw(20) << summary.maximum() << std::endl
              << "  Mean (s):              " << std::setw(20) << summary.mean() << std::endl
              << "  St. Dev.:              " << std::setw(20) << summary.standardDeviation() << std::endl
              << "Throughput envelope (operations per second):" << std::endl
              << "  Minimum (Hz):          " << std::setw(20) << minimumThroughput << std::endl
              << "  Maximum (Hz):          " << std::setw(20) << maximumThroughput << std::endl
              << "  Mean (Hz):             " << std::setw(20) << averageThroughput;
}

void logResults(const BenchmarkResult& result) {
    const auto maximum = std::max(
        result.baselineOuterLoopStatisticsSecondsPerIteration.mean(), result.comparisonOuterLoopStatisticsSecondsPerIteration.mean());
    const auto percentChangeLower = 100 * result.analysis.differenceConfidenceIntervalLowerLimit / maximum;
    const auto percentChangeUpper = 100 * result.analysis.differenceConfidenceIntervalUpperLimit / maximum;

    LOG(INFO) << std::fixed << std::setprecision(15) << std::setfill(' ') << "Statistics:" << std::endl
              << "  Baseline Implementation:" << std::endl
              << "    Count:                 " << result.baselineOuterLoopStatisticsSecondsPerIteration.count() << std::endl
              << "    Minimum (s):           " << std::setw(20) << result.baselineOuterLoopStatisticsSecondsPerIteration.minimum()
              << std::endl
              << "    Maximum (s):           " << std::setw(20) << result.baselineOuterLoopStatisticsSecondsPerIteration.maximum()
              << std::endl
              << "    Mean (s):              " << std::setw(20) << result.baselineOuterLoopStatisticsSecondsPerIteration.mean() << std::endl
              << "    St. Dev.:              " << std::setw(20) << result.baselineOuterLoopStatisticsSecondsPerIteration.standardDeviation()
              << std::endl
              << "  Comparison Implementation:" << std::endl
              << "    Count:                 " << result.comparisonOuterLoopStatisticsSecondsPerIteration.count() << std::endl
              << "    Minimum (s):           " << std::setw(20) << result.comparisonOuterLoopStatisticsSecondsPerIteration.minimum()
              << std::endl
              << "    Maximum (s):           " << std::setw(20) << result.comparisonOuterLoopStatisticsSecondsPerIteration.maximum()
              << std::endl
              << "    Mean (s):              " << std::setw(20) << result.comparisonOuterLoopStatisticsSecondsPerIteration.mean()
              << std::endl
              << "    St. Dev.:              " << std::setw(20)
              << result.comparisonOuterLoopStatisticsSecondsPerIteration.standardDeviation() << std::endl
              << "  Analysis:" << std::endl
              << "    T-statistic:           " << std::setw(40) << result.analysis.tStatistic << std::endl
              << "    Effective D.o.F.:      " << std::setw(40) << result.analysis.effectiveDegreesOfFreedom << std::endl
              << "    Pr[|t| > T]: by chance " << std::setw(40) << 100 * result.analysis.probabilityMeansEqual << "%" << std::endl
              << "    Difference 95% CI LL:  " << std::setw(40) << result.analysis.differenceConfidenceIntervalLowerLimit << std::endl
              << "    Difference 95% CI UL:  " << std::setw(40) << result.analysis.differenceConfidenceIntervalUpperLimit << std::endl
              << "  Likely % change:         [" << std::setw(40) << percentChangeLower << ", " << std::setw(40) << percentChangeUpper
              << "] %";
}

void compareSegmentBoxIntersection1() {
    constexpr size_t innerIterations = 150;
    const size_t outerIterations = std::thread::hardware_concurrency() * 100;

    LOG(INFO) << "Estimating timing using " << innerIterations << " inner iterations for each of " << outerIterations
              << " outer iterations." << std::endl;

    const auto result
        = performBenchmark(&baselineBoxSegment, &boxSegmentIntersectionNoReciprocalMultiply, outerIterations, innerIterations);
    logResults(result);
}

void compareSegmentBoxIntersection2() {
    constexpr size_t innerIterations = 150;
    const size_t outerIterations = std::thread::hardware_concurrency() * 100;

    LOG(INFO) << "Estimating timing using " << innerIterations << " inner iterations for each of " << outerIterations
              << " outer iterations.";

    const auto result = performBenchmark(&baselineBoxSegment, &boxSegmentIntersectionTavianator, outerIterations, innerIterations);
    logResults(result);
}

std::vector<std::array<float, 3> > samplePointsOnAxisAlignedBox(std::array<float, 3> boxCenter, float halfExtent) {
    constexpr int samplesPerFaceDimension = 1000;

    std::vector<std::array<float, 3> > result;
    result.reserve(samplesPerFaceDimension * samplesPerFaceDimension * 6);

    // Top face
    for (int x = 0; x < samplesPerFaceDimension; ++x) {
        const float xf = boxCenter[0] + halfExtent * (2.0f * x / samplesPerFaceDimension - 1.0f);

        for (int y = 0; y < samplesPerFaceDimension; ++y) {
            const float yf = boxCenter[1] + halfExtent * (2.0f * y / samplesPerFaceDimension - 1.0f);
            result.push_back(std::array<float, 3>{ { xf, yf, boxCenter[2] - halfExtent } });
        }
    }

    // Bottom face
    for (int x = 0; x < samplesPerFaceDimension; ++x) {
        const float xf = boxCenter[0] + halfExtent * (2.0f * x / samplesPerFaceDimension - 1.0f);

        for (int y = 0; y < samplesPerFaceDimension; ++y) {
            const float yf = boxCenter[1] + halfExtent * (2.0f * y / samplesPerFaceDimension - 1.0f);
            result.push_back(std::array<float, 3>{ { xf, yf, boxCenter[2] - halfExtent } });
        }
    }

    // Right face
    for (int y = 0; y < samplesPerFaceDimension; ++y) {
        const float yf = boxCenter[1] + halfExtent * (2.0f * y / samplesPerFaceDimension - 1.0f);
        for (int z = 0; z < samplesPerFaceDimension; ++z) {
            const float zf = boxCenter[2] + halfExtent * (2.0f * z / samplesPerFaceDimension - 1.0f);
            result.push_back(std::array<float, 3>{ { boxCenter[0] + halfExtent, yf, zf } });
        }
    }

    // Left face
    for (int y = 0; y < samplesPerFaceDimension; ++y) {
        const float yf = boxCenter[1] + halfExtent * (2.0f * y / samplesPerFaceDimension - 1.0f);
        for (int z = 0; z < samplesPerFaceDimension; ++z) {
            const float zf = boxCenter[2] + halfExtent * (2.0f * z / samplesPerFaceDimension - 1.0f);
            result.push_back(std::array<float, 3>{ { boxCenter[0] - halfExtent, yf, zf } });
        }
    }

    // Front face
    for (int x = 0; x < samplesPerFaceDimension; ++x) {
        const float xf = boxCenter[0] + halfExtent * (2.0f * x / samplesPerFaceDimension - 1.0f);
        for (int z = 0; z < samplesPerFaceDimension; ++z) {
            const float zf = boxCenter[2] + halfExtent * (2.0f * z / samplesPerFaceDimension - 1.0f);
            result.push_back(std::array<float, 3>{ { xf, boxCenter[1] + halfExtent, zf } });
        }
    }

    // Back face
    for (int x = 0; x < samplesPerFaceDimension; ++x) {
        const float xf = boxCenter[0] + halfExtent * (2.0f * x / samplesPerFaceDimension - 1.0f);
        for (int z = 0; z < samplesPerFaceDimension; ++z) {
            const float zf = boxCenter[2] + halfExtent * (2.0f * z / samplesPerFaceDimension - 1.0f);
            result.push_back(std::array<float, 3>{ { xf, boxCenter[1] - halfExtent, zf } });
        }
    }

    return result;
}

void baselineOccupiedBoxQueries() {
    // Why not a benchmark of occupied v. explicit free-space inserts? Because I don't have easy access to a decent ray-tracer.
    // For measuring performance in the occupied-only case this isn't a problem: points are points, regardless of the
    // perspective from which they've been perceived.
    constexpr float modelHalfExtent = 1;
    constexpr uint32_t modelDepth = 12;

    constexpr float halfExtentsPerBox = 4;

    constexpr int numBoxesX = 5;
    constexpr float boxHalfExtentX = modelHalfExtent / (halfExtentsPerBox * numBoxesX);
    constexpr int numBoxesY = 3;
    constexpr float boxHalfExtentY = modelHalfExtent / (halfExtentsPerBox * numBoxesY);
    constexpr int numBoxesZ = 2;
    constexpr float boxHalfExtentZ = modelHalfExtent / (halfExtentsPerBox * numBoxesZ);
    constexpr float boxHalfExtent = std::min(std::min(boxHalfExtentX, boxHalfExtentY), boxHalfExtentZ);
    constexpr float boxOffsetX = modelHalfExtent / (1 + numBoxesX);
    constexpr float boxOffsetY = modelHalfExtent / (1 + numBoxesY);
    constexpr float boxOffsetZ = modelHalfExtent / (1 + numBoxesZ);

    namespace dm = dense_mapping;
    namespace dmp = dm::payloads;
    namespace dmi = dm::insert_policies;
    namespace dmi = dm::insert_policies;
    using scalar_type = float;
    using octree_type = dm::Octree<scalar_type, dmp::OccupiedOnlyOctreeLeafNode, dmi::TraverseBoxInsertPolicy>;
    using point_type = std::array<scalar_type, 3>;
    using leaf_node_type = typename octree_type::leaf_node_type;

    SummaryStatistics<double> insertTimes;

    octree_type tree(modelDepth, modelHalfExtent);
    constexpr point_type zero{ { 0, 0, 0 } };
    for (int x = 0; x < numBoxesX; ++x) {
        for (int y = 0; y < numBoxesY; ++y) {
            for (int z = 0; z < numBoxesZ; ++z) {
                const point_type center{ { boxOffsetX / 2 + x * boxOffsetX - modelHalfExtent / 2,
                    boxOffsetY / 2 + y * boxOffsetY - modelHalfExtent / 2, boxOffsetZ / 2 + z * boxOffsetZ - modelHalfExtent / 2 } };
                const auto points = samplePointsOnAxisAlignedBox(center, boxHalfExtent);

                auto start = std::chrono::high_resolution_clock::now();
                for (auto p : points) {
                    tree.insert(zero, p);
                }
                const auto timePerInsert
                    = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count() / points.size();
                insertTimes.update(timePerInsert);
            }
        }
    }

    LOG(INFO) << "Insert time statistics";
    logResults(insertTimes);

    constexpr size_t numLoops = 1000;
    constexpr size_t queriesPerLoop = 1000;

    SummaryStatistics<double> queryTimes;

    std::uniform_real_distribution<scalar_type> pointDist(-modelHalfExtent, modelHalfExtent);
    std::uniform_real_distribution<scalar_type> extentDist(0, boxHalfExtent);

    uint64_t occupiedCount = 0;

    for (size_t l = 0; l < numLoops; ++l) {
        std::vector<dm::queries::AxisAlignedBoxQuery<scalar_type> > queries;
        queries.reserve(queriesPerLoop);

        for (size_t q = 0; q < queriesPerLoop; ++q) {
            const scalar_type queryHalfExtent = extentDist(prng);
            const point_type center{ { pointDist(prng), pointDist(prng), pointDist(prng) } };
            queries.emplace_back(dm::queries::AxisAlignedBoxQuery<scalar_type>(center, queryHalfExtent));
        }

        auto start = std::chrono::high_resolution_clock::now();
        for (const auto& q : queries) {
            tree.query(q, [&occupiedCount](const leaf_node_type& n) { occupiedCount += n.m_occupiedCount; });
        }

        const auto timePerQuery = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count() / queries.size();
        queryTimes.update(timePerQuery);
    }

    LOG(INFO) << "Points contained in queries: " << occupiedCount;
    LOG(INFO) << "Query timings";

    logResults(queryTimes);
}
}

int main(int, char**) {
    std::map<std::string, std::function<void()> > benchmarks;

    benchmarks["segmentIntersectsAxisAlignedBoundingBox v. segmentIntersectsAxisAlignedBoundingBox2"] = &compareSegmentBoxIntersection1;
    benchmarks["segmentIntersectsAxisAlignedBoundingBox v. tavianator"] = &compareSegmentBoxIntersection2;
    benchmarks["baselineOccupiedBoxQueries"] = &baselineOccupiedBoxQueries;

    for (auto& x : benchmarks) {
        auto start = std::chrono::high_resolution_clock::now();

        LOG(INFO) << "Starting benchmark [" << x.first << "]";
        x.second();
        std::chrono::duration<double> elapsed(std::chrono::high_resolution_clock::now() - start);
        LOG(INFO) << "Benchmark took " << elapsed.count() << "s to complete.";
    }

    return 0;
}
