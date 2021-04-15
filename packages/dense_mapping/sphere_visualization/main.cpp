#include "../include/insert_policies.h"
#include "../include/octree.h"
#include "../include/octree_payloads.h"
#include "glog/logging.h"
#include "packages/benchmarking/include/summary_statistics.h"
#include "sample_unit_sphere.h"
#include <chrono>
#include <iomanip>
#include <iostream>

namespace {
using scalar_type = float;

namespace dm = dense_mapping;
namespace dmp = dm::payloads;
namespace dmi = dm::insert_policies;
namespace dmi = dm::insert_policies;

using explicit_free_space_octree_type = dm::Octree<scalar_type, dmp::ExplicitFreeSpaceOctreeLeafNode, dmi::TraverseSegmentInsertPolicy>;
using occupied_only_octree_type = dm::Octree<scalar_type, dmp::OccupiedOnlyOctreeLeafNode, dmi::TraverseBoxInsertPolicy>;
using clock_type = std::chrono::high_resolution_clock;

constexpr scalar_type halfExtent = 4;
constexpr uint32_t depth = 10;
constexpr uint64_t maximumLeafNodes = 1ULL << (3 * depth);
constexpr size_t sphereAlphaSamples = 250;
constexpr size_t sphereThetaSamples = 250;

constexpr size_t viewPsiSamples = 10;
constexpr size_t viewThetaSamples = 10;

std::vector<std::pair<std::array<scalar_type, 3>, std::vector<std::array<scalar_type, 3> > > > generateSamples() {
    LOG(INFO) << "Generating [" << viewPsiSamples << " x " << viewThetaSamples << "] frames with [" << sphereAlphaSamples << " x "
              << sphereThetaSamples << "] rays each." << std::endl;
    const auto bottomView = samplePointsOnUnitSphere<scalar_type>(halfExtent, sphereAlphaSamples, sphereThetaSamples);

    std::vector<std::pair<std::array<scalar_type, 3>, std::vector<std::array<scalar_type, 3> > > > toInsert;
    for (size_t t = 0; t < viewThetaSamples; ++t) {
        const scalar_type theta = -M_PI + 2 * M_PI * t / viewThetaSamples;
        const auto ctheta = std::cos(theta);
        const auto stheta = std::sin(theta);
        const scalar_type rz_theta[3][3] = { { ctheta, stheta, 0 }, { -stheta, ctheta, 0 }, { 0, 0, 1 } };

        for (size_t p = 1; p < viewPsiSamples; ++p) {
            const scalar_type psi = -M_PI / 2 + M_PI * p / viewPsiSamples;
            const auto cpsi = std::cos(psi);
            const auto spsi = std::sin(psi);
            const scalar_type ry_psi[3][3] = { { cpsi, 0, spsi }, { 0, 1, 0 }, { -spsi, 0, cpsi } };

            // ry_psi * rz_theta
            const scalar_type r[3][3] = {
                { ry_psi[0][0] * rz_theta[0][0] + ry_psi[0][1] * rz_theta[1][0] + ry_psi[0][2] * rz_theta[2][0],
                    ry_psi[0][0] * rz_theta[0][1] + ry_psi[0][1] * rz_theta[1][1] + ry_psi[0][2] * rz_theta[2][1],
                    ry_psi[0][0] * rz_theta[0][2] + ry_psi[0][1] * rz_theta[1][2] + ry_psi[0][2] * rz_theta[2][2] },

                { ry_psi[1][0] * rz_theta[0][0] + ry_psi[1][1] * rz_theta[1][0] + ry_psi[1][2] * rz_theta[2][0],
                    ry_psi[1][0] * rz_theta[0][1] + ry_psi[1][1] * rz_theta[1][1] + ry_psi[1][2] * rz_theta[2][1],
                    ry_psi[1][0] * rz_theta[0][2] + ry_psi[1][1] * rz_theta[1][2] + ry_psi[1][2] * rz_theta[2][2] },

                { ry_psi[2][0] * rz_theta[0][0] + ry_psi[2][1] * rz_theta[1][0] + ry_psi[2][2] * rz_theta[2][0],
                    ry_psi[2][0] * rz_theta[0][1] + ry_psi[2][1] * rz_theta[1][1] + ry_psi[2][2] * rz_theta[2][1],
                    ry_psi[2][0] * rz_theta[0][2] + ry_psi[2][1] * rz_theta[1][2] + ry_psi[2][2] * rz_theta[2][2] },
            };

            std::pair<std::array<scalar_type, 3>, std::vector<std::array<scalar_type, 3> > > transformed;
            transformed.first[0] = r[0][0] * bottomView.first[0] + r[0][1] * bottomView.first[1] + r[0][2] * bottomView.first[2];
            transformed.first[1] = r[1][0] * bottomView.first[0] + r[1][1] * bottomView.first[1] + r[1][2] * bottomView.first[2];
            transformed.first[2] = r[2][0] * bottomView.first[0] + r[2][1] * bottomView.first[1] + r[2][2] * bottomView.first[2];

            for (const auto& orig : bottomView.second) {
                transformed.second.push_back(std::array<scalar_type, 3>{ {
                    r[0][0] * orig[0] + r[0][1] * orig[1] + r[0][2] * orig[2], r[1][0] * orig[0] + r[1][1] * orig[1] + r[1][2] * orig[2],
                    r[2][0] * orig[0] + r[2][1] * orig[1] + r[2][2] * orig[2],
                } });
            }

            toInsert.emplace_back(std::move(transformed));
        }
    }

    return toInsert;
}

template <typename OCTREE_TYPE>
OCTREE_TYPE populateOctree(const std::vector<std::pair<std::array<scalar_type, 3>, std::vector<std::array<scalar_type, 3> > > >& toInsert,
    SummaryStatistics<scalar_type>& timingStatistics, SummaryStatistics<scalar_type>& traversalStatistics) {
    OCTREE_TYPE result(halfExtent, depth);

    timingStatistics.clear();
    traversalStatistics.clear();

    for (const auto& frame : toInsert) {
        const auto start = clock_type::now();

        for (const auto& p : frame.second) {
            traversalStatistics.update(result.insert(frame.first, p));
        }

        const auto stop = clock_type::now();
        const auto elapsed = std::chrono::duration<scalar_type>(stop - start).count() / frame.second.size();
        timingStatistics.update(elapsed);
    }

    return result;
}

void logComparison(const std::string& a, const SummaryStatistics<scalar_type>& aStatistics, const std::string& b,
    const SummaryStatistics<scalar_type>& bStatistics) {
    const auto comparison = performWelchTest(aStatistics, bStatistics);
    const auto maximum = std::max(aStatistics.mean(), bStatistics.mean());
    const auto percentChangeLower = 100 * comparison.differenceConfidenceIntervalLowerLimit / maximum;
    const auto percentChangeUpper = 100 * comparison.differenceConfidenceIntervalUpperLimit / maximum;

    LOG(INFO) << std::fixed << std::setprecision(15) << std::setfill(' ') << "Statistics:" << std::endl
              << "  Baseline [" << a << "]:" << std::endl
              << "    Count:                 " << std::setw(20) << std::right << aStatistics.count() << std::endl
              << "    Minimum:               " << std::setw(20) << std::right << aStatistics.minimum() << std::endl
              << "    Maximum:               " << std::setw(20) << std::right << aStatistics.maximum() << std::endl
              << "    Mean:                  " << std::setw(20) << std::right << aStatistics.mean() << std::endl
              << "    St. Dev.:              " << std::setw(20) << std::right << aStatistics.standardDeviation() << std::endl
              << "  Comparison [" << b << "]:" << std::endl
              << "    Count:                 " << std::setw(20) << std::right << bStatistics.count() << std::endl
              << "    Minimum:               " << std::setw(20) << std::right << bStatistics.minimum() << std::endl
              << "    Maximum:               " << std::setw(20) << std::right << bStatistics.maximum() << std::endl
              << "    Mean:                  " << std::setw(20) << std::right << bStatistics.mean() << std::endl
              << "    St. Dev.:              " << std::setw(20) << bStatistics.standardDeviation() << std::endl
              << "  Analysis:" << std::endl
              << "    T-statistic:           " << std::setw(40) << std::right << comparison.tStatistic << std::endl
              << "    Effective D.o.F.:      " << std::setw(40) << std::right << comparison.effectiveDegreesOfFreedom << std::endl
              << "    Pr[|t| > T]: by chance " << std::setw(40) << std::right << 100 * comparison.probabilityMeansEqual << "%" << std::endl
              << "    Difference 95% CI LL:  " << std::setw(40) << std::right << comparison.differenceConfidenceIntervalLowerLimit
              << std::endl
              << "    Difference 95% CI UL:  " << std::setw(40) << std::right << comparison.differenceConfidenceIntervalUpperLimit
              << std::endl
              << "  Likely % change:         [" << std::setw(40) << std::right << percentChangeLower << ", " << std::setw(40) << std::right
              << percentChangeUpper << "] %" << std::endl;
}

void logTraversalStatistics(const std::string& description, const explicit_free_space_octree_type& tree) {
    size_t visitedNodes = 0;
    size_t whollyOccupiedNodes = 0;
    size_t partiallyOccupiedNodes = 0;
    size_t fullyEmptyNodes = 0;

    tree.visitAllLeaves([&](const explicit_free_space_octree_type::leaf_node_type& n) {
        ++visitedNodes;

        if (n.m_occupiedCount) {
            if (n.m_emptyCount) {
                ++partiallyOccupiedNodes;
            } else {
                ++whollyOccupiedNodes;
            }
        } else if (n.m_emptyCount) {
            ++fullyEmptyNodes;
        }
    });

    LOG(INFO) << std::fixed << std::setprecision(15) << std::setfill(' ') << "Occupancy statistics (leaf nodes) for configuration ["
              << description << "]: " << std::endl
              << "   Nodes visited:      " << visitedNodes << std::endl
              << "   Fully occupied:     " << whollyOccupiedNodes << " (" << (100.0 * whollyOccupiedNodes / visitedNodes)
              << "% of visited, " << (100.0 * whollyOccupiedNodes / maximumLeafNodes) << "% of all possible leaves)" << std::endl
              << "   Partially occupied: " << partiallyOccupiedNodes << " (" << (100.0 * partiallyOccupiedNodes / visitedNodes)
              << "% of visited, " << (100.0 * partiallyOccupiedNodes / maximumLeafNodes) << "% of all possible leaves)" << std::endl
              << "   Fully empty:        " << fullyEmptyNodes << " (" << (100.0 * fullyEmptyNodes / visitedNodes) << "% of visited, "
              << (100.0 * fullyEmptyNodes / maximumLeafNodes) << "% of all possible leaves)" << std::endl;
}

void logTraversalStatistics(const std::string& description, const occupied_only_octree_type& tree) {
    size_t visitedNodes = 0;

    tree.visitAllLeaves([&](const occupied_only_octree_type::leaf_node_type&) { ++visitedNodes; });

    LOG(INFO) << "Occupancy statistics (leaf nodes) for configuration [" << description << "]: " << std::endl
              << "   Nodes visited:      " << visitedNodes << std::endl;
}
}

int main(int, char**) {
    SummaryStatistics<scalar_type> freespaceTiming;
    SummaryStatistics<scalar_type> freespaceTraversals;
    SummaryStatistics<scalar_type> pointsTiming;
    SummaryStatistics<scalar_type> pointsTraversals;

    const auto data = generateSamples();
    const auto octree1 = populateOctree<explicit_free_space_octree_type>(data, freespaceTiming, freespaceTraversals);
    const auto octree2 = populateOctree<occupied_only_octree_type>(data, pointsTiming, pointsTraversals);

    logComparison("Insert with explicit free space, timing", freespaceTiming, "Insert point only, timing", pointsTiming);
    logComparison("Insert with explicit free space, leaf voxels traversed", freespaceTraversals, "Insert point only, leaf voxels traversed",
        pointsTraversals);
    logTraversalStatistics("Freespace", octree1);
    logTraversalStatistics("Points only", octree2);

    return 0;
}