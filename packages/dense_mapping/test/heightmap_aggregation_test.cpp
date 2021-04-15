#include "../include/insert_policies.h"
#include "../include/octree.h"
#include "../include/octree_payloads.h"
#include "../include/octree_queries.h"
#include "../include/query_aggregations.h"

#include "gtest/gtest.h"

#include <chrono>
#include <sstream>

namespace {
template <typename T> class HeightmapAggregationTest : public ::testing::Test {
public:
    using octree_type = dense_mapping::Octree<T, dense_mapping::payloads::OccupiedOnlyOctreeLeafNode,
        dense_mapping::insert_policies::TraverseBoxInsertPolicy>;
    using aabb_query_type = dense_mapping::queries::AxisAlignedBoxQuery<T>;
    using frustum_query_type = dense_mapping::queries::FrustumQuery<T>;
    using point_type = std::array<T, 3>;
    using aggregator_type = dense_mapping::aggregations::HeightMapAggregation<octree_type>;

    // Note:
    //
    // Before you change these values, make sure you double check what is going on with the tests below and ensure
    // that the appropriate concerns have been addressed.
    static constexpr T kHalfExtent = 1;
    static constexpr uint32_t kMaximumDepth = 10;

    static std::shared_ptr<octree_type> populatePlaneModel() {
        constexpr point_type origin{ { 0, 0, kHalfExtent } };
        constexpr int numSamples = 1000;

        auto result = std::make_shared<octree_type>(kHalfExtent, kMaximumDepth);

        for (int x = 0; x < numSamples; ++x) {
            // Want xx in the range (-kHalfExtent / 2) to (kHalfExtent / 2)
            const T xx = 2 * kHalfExtent * (x - numSamples / 2) / numSamples;

            for (int y = 0; y < numSamples; ++y) {
                const T yy = 2 * kHalfExtent * (y - numSamples / 2) / numSamples;

                T zz = -kHalfExtent;

                if (((xx >= -kHalfExtent / 2) && (xx <= kHalfExtent / 2)) && ((yy >= -kHalfExtent / 2) && (yy <= kHalfExtent / 2))) {
                    zz = 0;
                }

                point_type target{ { xx, yy, zz } };
                result->insert(origin, target);
            }
        }

        return result;
    }
};

template <typename T> constexpr T HeightmapAggregationTest<T>::kHalfExtent;

template <typename T> constexpr uint32_t HeightmapAggregationTest<T>::kMaximumDepth;

using TypesToTest = ::testing::Types<float, double>;

TYPED_TEST_CASE(HeightmapAggregationTest, TypesToTest);

TYPED_TEST(HeightmapAggregationTest, sanityCheckAggregationAABBQuery) {
    using query_type = typename TestFixture::aabb_query_type;
    using aggregator_type = typename TestFixture::aggregator_type;
    using point_type = typename TestFixture::point_type;

    auto octree = TestFixture::populatePlaneModel();

    constexpr point_type queryCenter{ { 0, 0, 0 } };
    constexpr TypeParam queryHalfExtent = TestFixture::kHalfExtent;
    constexpr point_type xAxis{ { 1, 0, 0 } };
    constexpr point_type yAxis{ { 0, 1, 0 } };

    query_type query(queryCenter, queryHalfExtent);
    aggregator_type aggregator(queryHalfExtent, queryHalfExtent, queryCenter, xAxis, yAxis);

    {
        const auto start = std::chrono::high_resolution_clock::now();
        octree->query(query, aggregator.m_visitor);
        const auto elapsed = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
        GTEST_LOG_(INFO) << "Time to perform query and generate aggregate: [" << elapsed << " s]";
    }

    aggregator.m_heightMap.prunePoints();
    constexpr int halfSample = 100;

    const auto expectedHeightInCenter = 4 * octree->volumeLength() / (1ULL << octree->leafDepth());
    const auto expectedHeightOutsideCenter = expectedHeightInCenter - octree->volumeLength();
    constexpr auto epsilon = std::numeric_limits<TypeParam>::epsilon();
    const auto delta = 8 * expectedHeightInCenter;
    constexpr auto expansionRatioNumerator = 12;
    constexpr auto expansionRatioDenominator = 10;

    for (int x = -halfSample; x < halfSample; ++x) {
        const auto xx = expansionRatioNumerator * queryHalfExtent * x / (expansionRatioDenominator * halfSample);

        for (int y = -halfSample; y < halfSample; ++y) {
            const auto yy = expansionRatioNumerator * queryHalfExtent * y / (expansionRatioDenominator * halfSample);
            const auto a = std::max(std::abs(xx), std::abs(yy));

            const auto estimatedHeight = aggregator.m_heightMap.estimateHeightAtLocation(xx, yy);

            if (a < queryHalfExtent / 2 - delta) {
                // Definitely inside center square
                EXPECT_NE(0, estimatedHeight.first);
                EXPECT_NEAR(expectedHeightInCenter, estimatedHeight.second, epsilon);
            } else if (a > queryHalfExtent + delta) {
                // Definitely outside the region entirely
                EXPECT_EQ(0, estimatedHeight.first);
                EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(), estimatedHeight.second);
            } else if ((a < queryHalfExtent - delta) && (a > queryHalfExtent / 2 + delta)) {
                // Inside the region but outside the center square
                EXPECT_NE(0, estimatedHeight.first);
                EXPECT_NEAR(expectedHeightOutsideCenter, estimatedHeight.second, epsilon) << "X : [" << x << ", " << xx << "], Y: [" << y
                                                                                          << ", " << yy << "]";
            }
        }
    }
}
}
