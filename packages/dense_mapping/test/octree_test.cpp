#include "../include/octree.h"
#include "../include/box_geometry.h"
#include "../include/insert_policies.h"
#include "../include/octree_payloads.h"
#include "../include/octree_queries.h"

#include "gtest/gtest.h"

#include "packages/benchmarking/include/summary_statistics.h"

#include <array>
#include <chrono>
#include <map>
#include <memory>
#include <set>

namespace {
namespace dm = dense_mapping;
namespace dmp = dm::payloads;
namespace dmi = dm::insert_policies;
namespace dmi = dm::insert_policies;

template <typename T> struct OctreeImplementation {
    using scalar_type = T;
    using octree_type = dm::Octree<T, dmp::ExplicitFreeSpaceOctreeLeafNode, dmi::TraverseSegmentInsertPolicy>;
    using point_type = std::array<T, 3>;
    using leaf_node_type = typename octree_type::leaf_node_type;

    static std::shared_ptr<octree_type> create(T halfExtent, uint32_t depth) { return std::make_shared<octree_type>(halfExtent, depth); }
};

template <typename IMPLEMENTATION_UNDER_TEST> class OctreeTest : public ::testing::Test {
public:
    using scalar_type = typename IMPLEMENTATION_UNDER_TEST::scalar_type;
};

using OctreeTypesToTest = ::testing::Types<OctreeImplementation<float>, OctreeImplementation<double> >;

TYPED_TEST_CASE(OctreeTest, OctreeTypesToTest);

TYPED_TEST(OctreeTest, instantiateThrowsOnInvalidDepth) { EXPECT_THROW(TypeParam::create(1, 22), std::runtime_error); }

TYPED_TEST(OctreeTest, instantiateThrowsOnInvalidHalfSpan) { EXPECT_THROW(TypeParam::create(0, 10), std::runtime_error); }

TYPED_TEST(OctreeTest, canInstantiate) { EXPECT_NO_THROW(TypeParam::create(16, 8)); }

TYPED_TEST(OctreeTest, insertSinglePoint) {
    constexpr typename TypeParam::scalar_type rootHalfExtent = 0.5;
    constexpr typename TypeParam::point_type rayOrigin{ { 0, 0, -20 * rootHalfExtent } };
    constexpr typename TypeParam::point_type rayTarget{ { 0, 0, 0 } };
    constexpr size_t maxDepth = 12;

    // Why? Because (0,0,0) is included in 8 leaf nodes (it is a common corner shared by all 8)
    constexpr size_t expectedOccupiedCount = 8;

    for (size_t depth = 2; depth < maxDepth; ++depth) {
        auto octree = TypeParam::create(rootHalfExtent, depth);
        EXPECT_NO_THROW(octree->insert(rayOrigin, rayTarget));

        size_t occupiedCount = 0;
        size_t unoccupiedCount = 0;
        size_t visitedCount = 0;
        auto visitor = [&](const typename TypeParam::leaf_node_type& node) {
            ++visitedCount;

            if (node.m_emptyCount) {
                ++unoccupiedCount;
                EXPECT_EQ(1, node.m_emptyCount);
                EXPECT_EQ(0, node.m_occupiedCount);
            }

            if (node.m_occupiedCount) {
                ++occupiedCount;

                EXPECT_EQ(1, node.m_occupiedCount);
                EXPECT_EQ(0, node.m_centroidX);
                EXPECT_EQ(0, node.m_centroidY);
                EXPECT_EQ(0, node.m_centroidZ);
                EXPECT_EQ(0, node.m_momentXX);
                EXPECT_EQ(0, node.m_momentXY);
                EXPECT_EQ(0, node.m_momentXZ);
                EXPECT_EQ(0, node.m_momentYY);
                EXPECT_EQ(0, node.m_momentYZ);
                EXPECT_EQ(0, node.m_momentZZ);
                EXPECT_EQ(0, node.m_averageViewDirectionX);
                EXPECT_EQ(0, node.m_averageViewDirectionY);
                EXPECT_EQ(1, node.m_averageViewDirectionZ);
            }
        };

        octree->visitAllLeaves(visitor);

        EXPECT_EQ(expectedOccupiedCount, occupiedCount) << " @ depth = " << depth;
    }
}

TYPED_TEST(OctreeTest, boxQueryHappyPath) {
    using scalar_type = typename TypeParam::scalar_type;
    using point_type = typename TypeParam::point_type;

    constexpr scalar_type rootHalfExtent = 0.5;
    constexpr point_type rayOrigin{ { 0, 0, -20 * rootHalfExtent } };
    constexpr point_type rayTarget{ { 0, 0, 0 } };
    constexpr size_t maxDepth = 15;

    // Why? Because (0,0,0) is included in 8 leaf nodes (it is a common corner shared by all 8)
    constexpr size_t expectedOccupiedCount = 8;

    for (size_t depth = 2; depth < maxDepth; ++depth) {
        auto octree = TypeParam::create(rootHalfExtent, depth);
        EXPECT_NO_THROW(octree->insert(rayOrigin, rayTarget));

        size_t occupiedCount = 0;
        size_t unoccupiedCount = 0;
        size_t visitedCount = 0;
        auto visitor = [&](const typename TypeParam::leaf_node_type& node, const point_type&, scalar_type) {
            ++visitedCount;

            if (node.m_emptyCount) {
                ++unoccupiedCount;
                EXPECT_EQ(1, node.m_emptyCount);
                EXPECT_EQ(0, node.m_occupiedCount);
            }

            if (node.m_occupiedCount) {
                ++occupiedCount;

                EXPECT_EQ(1, node.m_occupiedCount);
                EXPECT_EQ(0, node.m_centroidX);
                EXPECT_EQ(0, node.m_centroidY);
                EXPECT_EQ(0, node.m_centroidZ);
                EXPECT_EQ(0, node.m_momentXX);
                EXPECT_EQ(0, node.m_momentXY);
                EXPECT_EQ(0, node.m_momentXZ);
                EXPECT_EQ(0, node.m_momentYY);
                EXPECT_EQ(0, node.m_momentYZ);
                EXPECT_EQ(0, node.m_momentZZ);
                EXPECT_EQ(0, node.m_averageViewDirectionX);
                EXPECT_EQ(0, node.m_averageViewDirectionY);
                EXPECT_EQ(1, node.m_averageViewDirectionZ);
            }
        };

        dense_mapping::queries::AxisAlignedBoxQuery<typename TypeParam::scalar_type> q(rayTarget, rootHalfExtent / (1ULL << (depth)));
        octree->query(q, visitor);

        EXPECT_EQ(expectedOccupiedCount, occupiedCount) << " @ depth = " << depth;
    }
}

TYPED_TEST(OctreeTest, frustumQueryHappyPath) {
    using scalar_type = typename TypeParam::scalar_type;
    using point_type = typename TypeParam::point_type;
    constexpr scalar_type rootHalfExtent = 0.5;
    constexpr point_type rayOrigin{ { 0, 0, -20 * rootHalfExtent } };
    constexpr point_type rayTarget{ { 0, 0, 0 } };
    constexpr point_type zero{ { 0, 0, 0 } };

    constexpr size_t maxDepth = 6;

    // Why? Because (0,0,0) is included in 8 leaf nodes (it is a common corner shared by all 8)
    constexpr size_t expectedOccupiedCount = 8;

    for (size_t depth = 2; depth < maxDepth; ++depth) {
        auto octree = TypeParam::create(rootHalfExtent, depth);
        EXPECT_NO_THROW(octree->insert(rayOrigin, rayTarget));

        size_t occupiedCount = 0;
        size_t unoccupiedCount = 0;
        size_t visitedCount = 0;
        auto visitor = [&](const typename TypeParam::leaf_node_type& node, const point_type&, scalar_type) {
            ++visitedCount;

            if (node.m_emptyCount) {
                ++unoccupiedCount;
                EXPECT_EQ(1, node.m_emptyCount);
                EXPECT_EQ(0, node.m_occupiedCount);
            }

            if (node.m_occupiedCount) {
                ++occupiedCount;

                EXPECT_EQ(1, node.m_occupiedCount);
                EXPECT_EQ(0, node.m_centroidX);
                EXPECT_EQ(0, node.m_centroidY);
                EXPECT_EQ(0, node.m_centroidZ);
                EXPECT_EQ(0, node.m_momentXX);
                EXPECT_EQ(0, node.m_momentXY);
                EXPECT_EQ(0, node.m_momentXZ);
                EXPECT_EQ(0, node.m_momentYY);
                EXPECT_EQ(0, node.m_momentYZ);
                EXPECT_EQ(0, node.m_momentZZ);
                EXPECT_EQ(0, node.m_averageViewDirectionX);
                EXPECT_EQ(0, node.m_averageViewDirectionY);
                EXPECT_EQ(1, node.m_averageViewDirectionZ);
            }
        };

        constexpr typename TypeParam::scalar_type frustumExtent = 5 * M_PI / 180;
        const auto frustum
            = dense_mapping::Geometry::frustum<typename TypeParam::scalar_type>(rayOrigin, zero, frustumExtent, frustumExtent);
        dense_mapping::queries::FrustumQuery<typename TypeParam::scalar_type> q(frustum);
        octree->query(q, visitor);

        EXPECT_EQ(expectedOccupiedCount, occupiedCount) << " @ depth = " << depth;
    }
}

TYPED_TEST(OctreeTest, boxCenterFromIndexThrowsOnInvalidNibble1) {
    // Find this in the "base" case
    constexpr typename TypeParam::scalar_type halfExtent = 1;
    constexpr uint64_t index = 0x8887;
    EXPECT_THROW(TypeParam::octree_type::boxCenterFromIndex(index, halfExtent), std::runtime_error);
}

TYPED_TEST(OctreeTest, boxCenterFromIndexThrowsOnInvalidNibble2) {
    // Find this in a "recursive" case
    constexpr typename TypeParam::scalar_type halfExtent = 1;
    constexpr uint64_t index = 0x7888;
    EXPECT_THROW(TypeParam::octree_type::boxCenterFromIndex(index, halfExtent), std::runtime_error);
}

TYPED_TEST(OctreeTest, boxCenterFromIndexWithIndex0) {
    constexpr typename TypeParam::scalar_type halfExtent = 1;
    constexpr uint64_t index = 0;
    const auto center = TypeParam::octree_type::boxCenterFromIndex(index, halfExtent);
    EXPECT_EQ(0, center[0]);
    EXPECT_EQ(0, center[1]);
    EXPECT_EQ(0, center[2]);
}

TYPED_TEST(OctreeTest, boxCenterFromIndexDepth0) {
    // Base case
    constexpr typename TypeParam::scalar_type halfExtent = 1;
    constexpr auto magnitudeValue = halfExtent / 2;

    for (int i = 0; i < 8; ++i) {
        const uint64_t index = (0x8 | i);
        const auto center = TypeParam::octree_type::boxCenterFromIndex(index, halfExtent);

        EXPECT_EQ((i & 1) ? magnitudeValue : -magnitudeValue, center[0]) << "i = " << i << ", center=[" << center[0] << ", " << center[1]
                                                                         << ", " << center[2] << "]";
        EXPECT_EQ((i & 2) ? magnitudeValue : -magnitudeValue, center[1]) << "i = " << i << ", center=[" << center[0] << ", " << center[1]
                                                                         << ", " << center[2] << "]";
        EXPECT_EQ((i & 4) ? magnitudeValue : -magnitudeValue, center[2]) << "i = " << i << ", center=[" << center[0] << ", " << center[1]
                                                                         << ", " << center[2] << "]";
    }
}

TYPED_TEST(OctreeTest, boxCenterFromIndexDepth1) {
    using T = typename TypeParam::scalar_type;

    // Base case
    constexpr T halfExtent = 1;

    for (int i = 0; i < 8; ++i) {
        const uint64_t index1 = (0x8 | i);
        const std::array<T, 3> expectedCenterLevel0{ { ((i & 0x1) ? halfExtent : -halfExtent) / 2,
            ((i & 0x2) ? halfExtent : -halfExtent) / 2, ((i & 0x4) ? halfExtent : -halfExtent) / 2 } };

        for (int ii = 0; ii < 8; ++ii) {
            const uint64_t index2 = ((index1 << 4) | (0x8 | i));
            const std::array<T, 3> expectedCenterLevel1{ { expectedCenterLevel0[0] + ((i & 0x1) ? halfExtent : -halfExtent) / 4,
                expectedCenterLevel0[1] + ((i & 0x2) ? halfExtent : -halfExtent) / 4,
                expectedCenterLevel0[2] + ((i & 0x4) ? halfExtent : -halfExtent) / 4 } };

            const auto computedCenter = TypeParam::octree_type::boxCenterFromIndex(index2, halfExtent);

            EXPECT_EQ(expectedCenterLevel1[0], computedCenter[0]) << "i = " << i << ", ii = " << ii << ", computed = [" << computedCenter[0]
                                                                  << ", " << computedCenter[1] << ", " << computedCenter[2];
            EXPECT_EQ(expectedCenterLevel1[1], computedCenter[1]) << "i = " << i << ", ii = " << ii << ", computed = [" << computedCenter[0]
                                                                  << ", " << computedCenter[1] << ", " << computedCenter[2];
            EXPECT_EQ(expectedCenterLevel1[2], computedCenter[2]) << "i = " << i << ", ii = " << ii << ", computed = [" << computedCenter[0]
                                                                  << ", " << computedCenter[1] << ", " << computedCenter[2];
        }
    }
}

TYPED_TEST(OctreeTest, mergeThrowsOnDifferentExtent) {
    using octree_type = typename TypeParam::octree_type;
    constexpr typename TypeParam::scalar_type leftExtent = 16;
    constexpr typename TypeParam::scalar_type rightExtent = 15;
    constexpr uint32_t leftDepth = 16;
    constexpr uint32_t rightDepth = 16;
    octree_type left(leftExtent, leftDepth);
    octree_type right(rightExtent, rightDepth);
    EXPECT_THROW(left.merge(right), std::runtime_error);
    EXPECT_THROW(right.merge(left), std::runtime_error);
}

TYPED_TEST(OctreeTest, mergeThrowsOnDifferentDepth) {
    using octree_type = typename TypeParam::octree_type;
    constexpr typename TypeParam::scalar_type leftExtent = 16;
    constexpr typename TypeParam::scalar_type rightExtent = 16;
    constexpr uint32_t leftDepth = 16;
    constexpr uint32_t rightDepth = 14;
    octree_type left(leftExtent, leftDepth);
    octree_type right(rightExtent, rightDepth);
    EXPECT_THROW(left.merge(right), std::runtime_error);
    EXPECT_THROW(right.merge(left), std::runtime_error);
}

template <typename T>
void compareVoxelSets(const std::map<uint64_t, const dmp::ExplicitFreeSpaceOctreeLeafNode<T>*>& a,
    const std::map<uint64_t, const dmp::ExplicitFreeSpaceOctreeLeafNode<T>*>& b) {
    constexpr T epsilon = std::numeric_limits<T>::epsilon();

    std::set<uint64_t> missingFromA;
    std::set<uint64_t> missingFromB;
    auto aa = a.begin();
    auto bb = b.begin();

    while (aa != a.end() && bb != b.end()) {
        if (aa->first < bb->first) {
            missingFromB.insert(aa->first);
            ++aa;
        } else if (bb->first < aa->first) {
            missingFromA.insert(bb->first);
            ++bb;
        } else {
            EXPECT_EQ(aa->second->m_sortKey, aa->first);
            EXPECT_EQ(bb->second->m_sortKey, bb->first);
            EXPECT_EQ(aa->second->m_occupiedCount, bb->second->m_occupiedCount) << aa->first;
            EXPECT_EQ(aa->second->m_emptyCount, bb->second->m_emptyCount) << aa->first;
            EXPECT_NEAR(aa->second->m_centroidX, bb->second->m_centroidX, epsilon) << aa->first;
            EXPECT_NEAR(aa->second->m_centroidY, bb->second->m_centroidY, epsilon) << aa->first;
            EXPECT_NEAR(aa->second->m_centroidZ, bb->second->m_centroidZ, epsilon) << aa->first;
            EXPECT_NEAR(aa->second->m_momentXX, bb->second->m_momentXX, epsilon) << aa->first;
            EXPECT_NEAR(aa->second->m_momentXY, bb->second->m_momentXY, epsilon) << aa->first;
            EXPECT_NEAR(aa->second->m_momentXZ, bb->second->m_momentXZ, epsilon) << aa->first;
            EXPECT_NEAR(aa->second->m_momentYY, bb->second->m_momentYY, epsilon) << aa->first;
            EXPECT_NEAR(aa->second->m_momentYZ, bb->second->m_momentYZ, epsilon) << aa->first;
            EXPECT_NEAR(aa->second->m_momentZZ, bb->second->m_momentZZ, epsilon) << aa->first;
            EXPECT_NEAR(aa->second->m_averageViewDirectionX, aa->second->m_averageViewDirectionX, epsilon) << aa->first;
            EXPECT_NEAR(aa->second->m_averageViewDirectionY, aa->second->m_averageViewDirectionY, epsilon) << aa->first;
            EXPECT_NEAR(aa->second->m_averageViewDirectionZ, aa->second->m_averageViewDirectionZ, epsilon) << aa->first;
            ++aa;
            ++bb;
        }
    }

    while (aa != a.end()) {
        missingFromB.insert(aa->first);
        ++aa;
    }

    while (bb != b.end()) {
        missingFromA.insert(bb->first);
        ++bb;
    }

    EXPECT_EQ(0, missingFromA.size());
    EXPECT_EQ(0, missingFromB.size());
}

TYPED_TEST(OctreeTest, mergeVerifyEquivalentOctreeCells) {
    using octree_type = typename TypeParam::octree_type;
    using scalar_type = typename TypeParam::scalar_type;
    using point_type = typename TypeParam::point_type;
    using leaf_node_type = typename octree_type::leaf_node_type;

    constexpr int32_t xCount = 101;
    constexpr int32_t halfXCount = xCount / 2;
    constexpr int32_t yCount = 101;
    constexpr int32_t halfYCount = yCount / 2;
    constexpr scalar_type volumeExtent = 1;
    constexpr int32_t maximumDepth = 8;
    constexpr point_type origin{ { 0, 0, volumeExtent / (1LL << (maximumDepth - 1)) } };

    octree_type left(volumeExtent, maximumDepth);
    octree_type right(volumeExtent, maximumDepth);
    octree_type expected(volumeExtent, maximumDepth);

    GTEST_LOG_(INFO) << "Populating octrees";
    for (auto xIdx = -halfXCount; xIdx <= halfXCount; ++xIdx) {
        const auto x = volumeExtent * xIdx / halfXCount;

        for (auto yIdx = -halfYCount; yIdx <= halfYCount; ++yIdx) {
            const auto y = volumeExtent * yIdx / halfYCount;
            const point_type target{ { x, y, 0 } };

            if (x <= 0) {
                left.insert(origin, target);
            } else {
                right.insert(origin, target);
            }
            expected.insert(origin, target);
        }
    }

    std::map<uint64_t, const leaf_node_type*> expectedVoxels;
    expected.visitAllLeaves([&](const leaf_node_type& node) { expectedVoxels.insert(std::make_pair(node.m_sortKey, &node)); });
    GTEST_LOG_(INFO) << "Traversed [" << expectedVoxels.size() << "] voxels in original volume";
    GTEST_LOG_(INFO) << "Merging right into left";
    octree_type mergedLR = left.merge(right);
    std::map<uint64_t, const leaf_node_type*> mergedVoxels;
    mergedLR.visitAllLeaves([&](const leaf_node_type& node) { mergedVoxels.insert(std::make_pair(node.m_sortKey, &node)); });
    GTEST_LOG_(INFO) << "Traversed [" << expectedVoxels.size() << "] voxels in merged volume";

    GTEST_LOG_(INFO) << "Comparing voxel sets from merged and original";
    compareVoxelSets(expectedVoxels, mergedVoxels);

    GTEST_LOG_(INFO) << "Merging left into right and checking";
    mergedVoxels.clear();
    octree_type mergedRL = right.merge(left);
    mergedRL.visitAllLeaves([&](const leaf_node_type& node) { mergedVoxels.insert(std::make_pair(node.m_sortKey, &node)); });

    compareVoxelSets(expectedVoxels, mergedVoxels);
}

TYPED_TEST(OctreeTest, mergeVerifyEquivalentAABBQueryResults) {
    using octree_type = typename TypeParam::octree_type;
    using scalar_type = typename TypeParam::scalar_type;
    using point_type = typename TypeParam::point_type;
    using leaf_node_type = typename octree_type::leaf_node_type;
    using query_type = dense_mapping::queries::AxisAlignedBoxQuery<scalar_type>;

    constexpr int32_t xCount = 101;
    constexpr int32_t halfXCount = xCount / 2;
    constexpr int32_t yCount = 101;
    constexpr int32_t halfYCount = yCount / 2;
    constexpr scalar_type volumeExtent = 1;
    constexpr int32_t maximumDepth = 8;
    constexpr point_type origin{ { 0, 0, volumeExtent / (1LL << (maximumDepth - 1)) } };

    octree_type left(volumeExtent, maximumDepth);
    octree_type right(volumeExtent, maximumDepth);
    octree_type expected(volumeExtent, maximumDepth);

    GTEST_LOG_(INFO) << "Populating octrees";
    for (auto xIdx = -halfXCount; xIdx <= halfXCount; ++xIdx) {
        const auto x = volumeExtent * xIdx / halfXCount;

        for (auto yIdx = -halfYCount; yIdx <= halfYCount; ++yIdx) {
            const auto y = volumeExtent * yIdx / halfYCount;
            const point_type target{ { x, y, 0 } };

            if (x <= 0) {
                left.insert(origin, target);
            } else {
                right.insert(origin, target);
            }
            expected.insert(origin, target);
        }
    }

    GTEST_LOG_(INFO) << "Merging octrees";
    auto merged = left.merge(right);

    constexpr int32_t queryXCount = 11;
    constexpr int32_t halfQueryXCount = queryXCount / 2;
    constexpr int32_t queryYCount = 11;
    constexpr int32_t halfQueryYCount = queryYCount / 2;
    constexpr scalar_type queryVolumeExtent = 4 * volumeExtent / (1ULL << maximumDepth);

    SummaryStatistics<double> queryAgainstExpectedTimes;
    SummaryStatistics<double> queryAgainstMergedTimes;

    for (auto xIdx = -halfQueryXCount; xIdx <= halfQueryXCount; ++xIdx) {
        const auto x = volumeExtent * xIdx / halfQueryXCount;

        for (auto yIdx = -halfQueryYCount; yIdx <= halfQueryYCount; ++yIdx) {
            const auto y = volumeExtent * yIdx / halfQueryYCount;
            const point_type target{ { x, y, 0 } };
            const query_type query(target, queryVolumeExtent);

            std::map<uint64_t, const leaf_node_type*> expectedVoxels;
            {
                const auto start = std::chrono::high_resolution_clock::now();
                expected.query(query, [&](const leaf_node_type& node) { expectedVoxels.insert(std::make_pair(node.m_sortKey, &node)); });
                queryAgainstExpectedTimes.update(std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count());
            }

            std::map<uint64_t, const leaf_node_type*> mergedVoxels;
            {
                const auto start = std::chrono::high_resolution_clock::now();
                merged.query(query, [&](const leaf_node_type& node) { mergedVoxels.insert(std::make_pair(node.m_sortKey, &node)); });
                queryAgainstMergedTimes.update(std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count());
            }

            compareVoxelSets(expectedVoxels, mergedVoxels);
        }
    }

    const auto welchTest = performWelchTest(queryAgainstExpectedTimes, queryAgainstMergedTimes);
    const auto den = std::max(std::abs(queryAgainstMergedTimes.mean()), std::abs(queryAgainstExpectedTimes.mean()));
    const auto minimumPercentChange = 100 * welchTest.differenceConfidenceIntervalLowerLimit / den;
    const auto maximumPercentChange = 100 * welchTest.differenceConfidenceIntervalUpperLimit / den;
    const auto expectedDifferent = queryAgainstExpectedTimes.mean() - queryAgainstMergedTimes.mean();

    GTEST_LOG_(INFO) << "Query timing for directly constructed octree:\n"
                     << queryAgainstExpectedTimes << "Query timing for merged octree:\n"
                     << queryAgainstMergedTimes << welchTest << "\n"
                     << "Difference in means: " << expectedDifferent << " (" << (100 * expectedDifferent / den) << "%)\n"
                     << "% difference:        " << minimumPercentChange << " / " << maximumPercentChange << "%";
}
}
