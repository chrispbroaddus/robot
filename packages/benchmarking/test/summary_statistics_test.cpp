#include "packages/benchmarking/include/summary_statistics.h"
#include "gtest/gtest.h"
#include <algorithm>
#include <array>
#include <numeric>
#include <random>
#include <vector>

namespace {
// https://en.wikipedia.org/wiki/Kahan_summation_algorithm
// This is the "best" way I know of trying to do this
template <typename CONST_ITERATOR_TYPE>
typename CONST_ITERATOR_TYPE::value_type computeMeanKahanSummation(CONST_ITERATOR_TYPE begin, CONST_ITERATOR_TYPE end) {
    typename CONST_ITERATOR_TYPE::value_type sum(0);
    typename CONST_ITERATOR_TYPE::value_type c(0);
    size_t n = 0;

    while (begin != end) {
        const auto x = *begin;
        const auto y = x - c;
        const auto t = sum + y;
        c = (t - sum) - y;
        sum = t;
        ++n;
        ++begin;
    }

    return sum / (n > 0 ? n : 1);
}

// https://en.wikipedia.org/wiki/Kahan_summation_algorithm
// This is the "best" way I know of trying to do this
template <typename CONST_ITERATOR_TYPE>
typename CONST_ITERATOR_TYPE::value_type computeVarianceKahanSummation(
    typename CONST_ITERATOR_TYPE::value_type mean, CONST_ITERATOR_TYPE begin, CONST_ITERATOR_TYPE end) {
    typename CONST_ITERATOR_TYPE::value_type sum(0);
    typename CONST_ITERATOR_TYPE::value_type c(0);
    size_t n = 0;

    while (begin != end) {
        const auto x = *begin - mean;
        const auto y = x * x - c;
        const auto t = sum + y;
        c = (t - sum) - y;
        sum = t;
        ++n;
        ++begin;
    }

    return sum / (n > 1 ? (n - 1) : 1);
}

template <typename T> std::vector<T> drawSample(size_t numSamples, std::mt19937::result_type seed) {
    std::mt19937 prng(seed);
    std::normal_distribution<T> dist(0, 1);

    std::vector<T> result;
    std::generate_n(std::back_inserter(result), numSamples, [&]() { return dist(prng); });
    return result;
}

typedef ::testing::Types<float, double> TypesToTest;

template <typename T> class SummaryStatisticsTest : public ::testing::Test {};

TYPED_TEST_CASE(SummaryStatisticsTest, TypesToTest);

TYPED_TEST(SummaryStatisticsTest, saneStateWithZeroSamples) {
    SummaryStatistics<TypeParam> stats;
    EXPECT_EQ(0UL, stats.count());
    EXPECT_EQ(0, stats.mean());
    EXPECT_EQ(0, stats.variance());
    EXPECT_EQ(0, stats.standardDeviation());
    EXPECT_EQ(std::numeric_limits<TypeParam>::infinity(), stats.minimum());
    EXPECT_EQ(-std::numeric_limits<TypeParam>::infinity(), stats.maximum());
}

TYPED_TEST(SummaryStatisticsTest, saneStateWithSingleSample) {
    SummaryStatistics<TypeParam> stats;
    stats.update(1);
    EXPECT_EQ(1UL, stats.count());
    EXPECT_EQ(1, stats.mean());
    EXPECT_EQ(0, stats.variance());
    EXPECT_EQ(0, stats.standardDeviation());
    EXPECT_EQ(1, stats.minimum());
    EXPECT_EQ(1, stats.maximum());
}

TYPED_TEST(SummaryStatisticsTest, singleSummaryHappyPath) {
    constexpr auto epsilon = 40 * std::numeric_limits<TypeParam>::epsilon();

    SummaryStatistics<TypeParam> stats;
    constexpr size_t expectedCount = 10000;
    const auto samples = drawSample<TypeParam>(expectedCount, 1);

    TypeParam expectedMaximum = -std::numeric_limits<TypeParam>::infinity();
    TypeParam expectedMinimum = std::numeric_limits<TypeParam>::infinity();
    for (auto s : samples) {
        stats.update(s);
        expectedMaximum = std::max(expectedMaximum, s);
        expectedMinimum = std::min(expectedMinimum, s);
    }

    const auto expectedMean = computeMeanKahanSummation(samples.begin(), samples.end());
    const auto expectedVariance = computeVarianceKahanSummation(expectedMean, samples.begin(), samples.end());
    EXPECT_NEAR(expectedMean, stats.mean(), std::numeric_limits<TypeParam>::epsilon());
    EXPECT_NEAR(expectedVariance, stats.variance(), epsilon);
    EXPECT_NEAR(std::sqrt(expectedVariance), stats.standardDeviation(), epsilon);
    EXPECT_EQ(expectedCount, stats.count());
    EXPECT_EQ(expectedMaximum, stats.maximum());
    EXPECT_EQ(expectedMinimum, stats.minimum());
}

TYPED_TEST(SummaryStatisticsTest, checkMergeEmptyIntoNonempty) {
    constexpr auto epsilon = 40 * std::numeric_limits<TypeParam>::epsilon();
    SummaryStatistics<TypeParam> stats1;
    constexpr size_t expectedCount = 10000;
    const auto samples = drawSample<TypeParam>(expectedCount, 1);

    TypeParam expectedMaximum = -std::numeric_limits<TypeParam>::infinity();
    TypeParam expectedMinimum = std::numeric_limits<TypeParam>::infinity();
    for (auto s : samples) {
        stats1.update(s);
        expectedMaximum = std::max(expectedMaximum, s);
        expectedMinimum = std::min(expectedMinimum, s);
    }

    const auto expectedMean = computeMeanKahanSummation(samples.begin(), samples.end());
    const auto expectedVariance = computeVarianceKahanSummation(expectedMean, samples.begin(), samples.end());
    EXPECT_NEAR(expectedMean, stats1.mean(), std::numeric_limits<TypeParam>::epsilon());
    EXPECT_NEAR(expectedVariance, stats1.variance(), epsilon);
    EXPECT_NEAR(std::sqrt(expectedVariance), stats1.standardDeviation(), epsilon);
    EXPECT_EQ(expectedCount, stats1.count());
    EXPECT_EQ(expectedMaximum, stats1.maximum());
    EXPECT_EQ(expectedMinimum, stats1.minimum());

    SummaryStatistics<TypeParam> stats2;
    SummaryStatistics<TypeParam> stats3(stats1, stats2);

    EXPECT_NEAR(expectedMean, stats3.mean(), std::numeric_limits<TypeParam>::epsilon());
    EXPECT_NEAR(expectedVariance, stats3.variance(), epsilon);
    EXPECT_NEAR(std::sqrt(expectedVariance), stats3.standardDeviation(), epsilon);
    EXPECT_EQ(expectedCount, stats3.count());
    EXPECT_EQ(expectedMaximum, stats3.maximum());
    EXPECT_EQ(expectedMinimum, stats3.minimum());
}

TYPED_TEST(SummaryStatisticsTest, checkMergeNonemptyIntoEmpty) {
    constexpr auto epsilon = 40 * std::numeric_limits<TypeParam>::epsilon();
    SummaryStatistics<TypeParam> stats1;
    constexpr size_t expectedCount = 10000;
    const auto samples = drawSample<TypeParam>(expectedCount, 1);

    TypeParam expectedMaximum = -std::numeric_limits<TypeParam>::infinity();
    TypeParam expectedMinimum = std::numeric_limits<TypeParam>::infinity();
    for (auto s : samples) {
        stats1.update(s);
        expectedMaximum = std::max(expectedMaximum, s);
        expectedMinimum = std::min(expectedMinimum, s);
    }

    const auto expectedMean = computeMeanKahanSummation(samples.begin(), samples.end());
    const auto expectedVariance = computeVarianceKahanSummation(expectedMean, samples.begin(), samples.end());
    EXPECT_NEAR(expectedMean, stats1.mean(), std::numeric_limits<TypeParam>::epsilon());
    EXPECT_NEAR(expectedVariance, stats1.variance(), epsilon);
    EXPECT_NEAR(std::sqrt(expectedVariance), stats1.standardDeviation(), epsilon);
    EXPECT_EQ(expectedCount, stats1.count());
    EXPECT_EQ(expectedMaximum, stats1.maximum());
    EXPECT_EQ(expectedMinimum, stats1.minimum());

    SummaryStatistics<TypeParam> stats2;
    // Reverse order of above
    SummaryStatistics<TypeParam> stats3(stats2, stats1);

    EXPECT_NEAR(expectedMean, stats3.mean(), std::numeric_limits<TypeParam>::epsilon());
    EXPECT_NEAR(expectedVariance, stats3.variance(), epsilon);
    EXPECT_NEAR(std::sqrt(expectedVariance), stats3.standardDeviation(), epsilon);
    EXPECT_EQ(expectedCount, stats3.count());
    EXPECT_EQ(expectedMaximum, stats3.maximum());
    EXPECT_EQ(expectedMinimum, stats3.minimum());
}

TYPED_TEST(SummaryStatisticsTest, checkMergeNonemptyIntoNonemptySameSizes) {
    constexpr auto epsilon = 40 * std::numeric_limits<TypeParam>::epsilon();
    constexpr size_t expectedCount = 10000;
    SummaryStatistics<TypeParam> stats1;
    const auto samples1 = drawSample<TypeParam>(expectedCount, 1);

    TypeParam expectedMaximum1 = -std::numeric_limits<TypeParam>::infinity();
    TypeParam expectedMinimum1 = std::numeric_limits<TypeParam>::infinity();
    for (auto s : samples1) {
        stats1.update(s);
        expectedMaximum1 = std::max(expectedMaximum1, s);
        expectedMinimum1 = std::min(expectedMinimum1, s);
    }

    const auto expectedMean1 = computeMeanKahanSummation(samples1.begin(), samples1.end());
    const auto expectedVariance1 = computeVarianceKahanSummation(expectedMean1, samples1.begin(), samples1.end());
    EXPECT_NEAR(expectedMean1, stats1.mean(), std::numeric_limits<TypeParam>::epsilon());
    EXPECT_NEAR(expectedVariance1, stats1.variance(), epsilon);
    EXPECT_NEAR(std::sqrt(expectedVariance1), stats1.standardDeviation(), epsilon);
    EXPECT_EQ(expectedCount, stats1.count());
    EXPECT_EQ(expectedMaximum1, stats1.maximum());
    EXPECT_EQ(expectedMinimum1, stats1.minimum());

    SummaryStatistics<TypeParam> stats2;
    const auto samples2 = drawSample<TypeParam>(expectedCount, 2);

    TypeParam expectedMaximum2 = -std::numeric_limits<TypeParam>::infinity();
    TypeParam expectedMinimum2 = std::numeric_limits<TypeParam>::infinity();
    for (auto s : samples2) {
        stats2.update(s);
        expectedMaximum2 = std::max(expectedMaximum2, s);
        expectedMinimum2 = std::min(expectedMinimum2, s);
    }

    const auto expectedMean2 = computeMeanKahanSummation(samples2.begin(), samples2.end());
    const auto expectedVariance2 = computeVarianceKahanSummation(expectedMean2, samples2.begin(), samples2.end());
    EXPECT_NEAR(expectedMean2, stats2.mean(), 2 * std::numeric_limits<TypeParam>::epsilon());
    EXPECT_NEAR(expectedVariance2, stats2.variance(), epsilon);
    EXPECT_NEAR(std::sqrt(expectedVariance2), stats2.standardDeviation(), epsilon);
    EXPECT_EQ(expectedCount, stats2.count());
    EXPECT_EQ(expectedMaximum2, stats2.maximum());
    EXPECT_EQ(expectedMinimum2, stats2.minimum());

    const TypeParam expectedMaximum3 = std::max(expectedMaximum1, expectedMaximum2);
    const TypeParam expectedMinimum3 = std::min(expectedMinimum1, expectedMinimum2);
    std::vector<TypeParam> samples3(samples1);
    std::copy(samples2.begin(), samples2.end(), std::back_inserter(samples3));

    SummaryStatistics<TypeParam> stats3(stats2, stats1);

    const auto expectedMean3 = computeMeanKahanSummation(samples3.begin(), samples3.end());
    const auto expectedVariance3 = computeVarianceKahanSummation(expectedMean3, samples3.begin(), samples3.end());
    EXPECT_NEAR(expectedMean3, stats3.mean(), std::numeric_limits<TypeParam>::epsilon());
    EXPECT_NEAR(expectedVariance3, stats3.variance(), epsilon);
    EXPECT_NEAR(std::sqrt(expectedVariance3), stats3.standardDeviation(), epsilon);
    EXPECT_EQ(2 * expectedCount, stats3.count());
    EXPECT_EQ(expectedMaximum3, stats3.maximum());
    EXPECT_EQ(expectedMinimum3, stats3.minimum());
}

TYPED_TEST(SummaryStatisticsTest, checkMergeNonemptyIntoNonemptyDifferentSizes) {
    constexpr auto epsilon = 40 * std::numeric_limits<TypeParam>::epsilon();
    constexpr size_t expectedCount1 = 10000;
    constexpr size_t expectedCount2 = 2 * expectedCount1;

    SummaryStatistics<TypeParam> stats1;
    const auto samples1 = drawSample<TypeParam>(expectedCount1, 1);

    TypeParam expectedMaximum1 = -std::numeric_limits<TypeParam>::infinity();
    TypeParam expectedMinimum1 = std::numeric_limits<TypeParam>::infinity();
    for (auto s : samples1) {
        stats1.update(s);
        expectedMaximum1 = std::max(expectedMaximum1, s);
        expectedMinimum1 = std::min(expectedMinimum1, s);
    }

    const auto expectedMean1 = computeMeanKahanSummation(samples1.begin(), samples1.end());
    const auto expectedVariance1 = computeVarianceKahanSummation(expectedMean1, samples1.begin(), samples1.end());
    EXPECT_NEAR(expectedMean1, stats1.mean(), std::numeric_limits<TypeParam>::epsilon());
    EXPECT_NEAR(expectedVariance1, stats1.variance(), epsilon);
    EXPECT_NEAR(std::sqrt(expectedVariance1), stats1.standardDeviation(), epsilon);
    EXPECT_EQ(expectedCount1, stats1.count());
    EXPECT_EQ(expectedMaximum1, stats1.maximum());
    EXPECT_EQ(expectedMinimum1, stats1.minimum());

    SummaryStatistics<TypeParam> stats2;
    const auto samples2 = drawSample<TypeParam>(expectedCount2, 2);

    TypeParam expectedMaximum2 = -std::numeric_limits<TypeParam>::infinity();
    TypeParam expectedMinimum2 = std::numeric_limits<TypeParam>::infinity();
    for (auto s : samples2) {
        stats2.update(s);
        expectedMaximum2 = std::max(expectedMaximum2, s);
        expectedMinimum2 = std::min(expectedMinimum2, s);
    }

    const auto expectedMean2 = computeMeanKahanSummation(samples2.begin(), samples2.end());
    const auto expectedVariance2 = computeVarianceKahanSummation(expectedMean2, samples2.begin(), samples2.end());
    EXPECT_NEAR(expectedMean2, stats2.mean(), std::numeric_limits<TypeParam>::epsilon());
    EXPECT_NEAR(expectedVariance2, stats2.variance(), epsilon);
    EXPECT_NEAR(std::sqrt(expectedVariance2), stats2.standardDeviation(), epsilon);
    EXPECT_EQ(expectedCount2, stats2.count());
    EXPECT_EQ(expectedMaximum2, stats2.maximum());
    EXPECT_EQ(expectedMinimum2, stats2.minimum());

    const auto expectedCount3 = expectedCount1 + expectedCount2;
    const TypeParam expectedMaximum3 = std::max(expectedMaximum1, expectedMaximum2);
    const TypeParam expectedMinimum3 = std::min(expectedMinimum1, expectedMinimum2);
    std::vector<TypeParam> samples3(samples1);
    std::copy(samples2.begin(), samples2.end(), std::back_inserter(samples3));

    SummaryStatistics<TypeParam> stats3(stats2, stats1);

    const auto expectedMean3 = computeMeanKahanSummation(samples3.begin(), samples3.end());
    const auto expectedVariance3 = computeVarianceKahanSummation(expectedMean3, samples3.begin(), samples3.end());
    EXPECT_NEAR(expectedMean3, stats3.mean(), std::numeric_limits<TypeParam>::epsilon());
    EXPECT_NEAR(expectedVariance3, stats3.variance(), epsilon);
    EXPECT_NEAR(std::sqrt(expectedVariance3), stats3.standardDeviation(), epsilon);
    EXPECT_EQ(expectedCount3, stats3.count());
    EXPECT_EQ(expectedMaximum3, stats3.maximum());
    EXPECT_EQ(expectedMinimum3, stats3.minimum());
}

TYPED_TEST(SummaryStatisticsTest, sanityCheckCumulativeStudentTDistribution) {
    constexpr size_t M = 10;
    constexpr size_t N = 11;

    // Not super tight tolerances but given that everything we're doing here is super approximate anyways, this
    // serves as a reasonable sanity check.
    constexpr TypeParam epsilon = 7.5e-3;

    // Taken from https://en.wikipedia.org/wiki/Student%27s_t-distribution#Table_of_selected_values
    constexpr std::array<TypeParam, N> twoSidedProbabilities{ { 0.50, 0.60, 0.70, 0.80, 0.90, 0.95, 0.98, 0.99, 0.995, 0.998, 0.999 } };

    // Taken from https://en.wikipedia.org/wiki/Student%27s_t-distribution#Table_of_selected_values
    constexpr std::array<std::array<TypeParam, N>, M> cutoffs{ {
        { { 1.000, 1.376, 1.963, 3.078, 6.314, 12.71, 31.82, 63.66, 127.3, 318.3, 636.6 } }, // DF = 1
        { { 0.816, 1.080, 1.386, 1.886, 2.920, 4.303, 6.965, 9.925, 14.09, 22.33, 31.60 } }, // DF = 2
        { { 0.765, 0.978, 1.250, 1.638, 2.353, 3.182, 4.541, 5.841, 7.453, 10.21, 12.92 } }, // DF = 3
        { { 0.741, 0.941, 1.190, 1.533, 2.132, 2.776, 3.747, 4.604, 5.598, 7.173, 8.610 } }, // DF = 4
        { { 0.727, 0.920, 1.156, 1.476, 2.015, 2.571, 3.365, 4.032, 4.773, 5.893, 6.869 } }, // DF = 5
        { { 0.718, 0.906, 1.134, 1.440, 1.943, 2.447, 3.143, 3.707, 4.317, 5.208, 5.959 } }, // DF = 6
        { { 0.711, 0.896, 1.119, 1.415, 1.895, 2.365, 2.998, 3.499, 4.029, 4.785, 5.408 } }, // DF = 7
        { { 0.706, 0.889, 1.108, 1.397, 1.860, 2.306, 2.896, 3.355, 3.833, 4.501, 5.041 } }, // DF = 8
        { { 0.703, 0.883, 1.100, 1.383, 1.833, 2.262, 2.821, 3.250, 3.690, 4.297, 4.781 } }, // DF = 9
        { { 0.700, 0.879, 1.093, 1.372, 1.812, 2.228, 2.764, 3.169, 3.581, 4.144, 4.587 } }, // DF = 10
    } };
    for (size_t i = 0; i < cutoffs.size(); ++i) {
        const TypeParam degreesOfFreedom = 1 + i;

        const auto& values = cutoffs.at(i);
        for (size_t j = 0; j < twoSidedProbabilities.size(); ++j) {
            const TypeParam t = values.at(j);
            const TypeParam expectedValue = twoSidedProbabilities[j];
            const TypeParam computedValue = details::cumulativeStudentTDistribution(t, degreesOfFreedom);
            EXPECT_NEAR(expectedValue, computedValue, epsilon);
        }
    }
}

TYPED_TEST(SummaryStatisticsTest, exerciseWelchTTestOne) {
    constexpr size_t expectedCount = 10000;
    SummaryStatistics<TypeParam> stats1;
    const auto samples1 = drawSample<TypeParam>(expectedCount, 1);

    for (auto s : samples1) {
        stats1.update(s);
    }

    SummaryStatistics<TypeParam> stats2;
    const auto samples2 = drawSample<TypeParam>(expectedCount, 5);

    for (auto s : samples2) {
        stats2.update(s);
    }

    const auto test = performWelchTest(stats1, stats2);
    GTEST_LOG_(INFO) << "Test: T=" << test.tStatistic << ", nu=" << test.effectiveDegreesOfFreedom
                     << ", Pr[t < -T | T < t]=" << test.probabilityMeansEqual << ", CI=[" << test.differenceConfidenceIntervalLowerLimit
                     << ", " << test.differenceConfidenceIntervalUpperLimit << "]";
}

TYPED_TEST(SummaryStatisticsTest, exerciseWelchTTestTwo) {
    constexpr size_t expectedCount = 10000;
    SummaryStatistics<TypeParam> stats1;
    const auto samples1 = drawSample<TypeParam>(expectedCount, 1);

    for (auto s : samples1) {
        stats1.update(s);
    }

    SummaryStatistics<TypeParam> stats2;
    const auto samples2 = drawSample<TypeParam>(expectedCount, 2);

    for (auto s : samples2) {
        stats2.update(1 + s);
    }

    const auto test = performWelchTest(stats1, stats2);
    GTEST_LOG_(INFO) << "Test: T=" << test.tStatistic << ", nu=" << test.effectiveDegreesOfFreedom
                     << ", Pr[t < -T | T < t]=" << test.probabilityMeansEqual << ", CI=[" << test.differenceConfidenceIntervalLowerLimit
                     << ", " << test.differenceConfidenceIntervalUpperLimit << "]";
}

TYPED_TEST(SummaryStatisticsTest, exerciseWelchTTestThree) {
    constexpr size_t expectedCount = 10000;
    SummaryStatistics<TypeParam> stats1;
    const auto samples1 = drawSample<TypeParam>(expectedCount, 1);

    for (auto s : samples1) {
        stats1.update(s);
    }

    SummaryStatistics<TypeParam> stats2;
    const auto samples2 = drawSample<TypeParam>(expectedCount, 2);

    for (auto s : samples2) {
        stats2.update(5 + s);
    }

    const auto test = performWelchTest(stats1, stats2);
    GTEST_LOG_(INFO) << "Test: T=" << test.tStatistic << ", nu=" << test.effectiveDegreesOfFreedom
                     << ", Pr[t < -T | T < t]=" << test.probabilityMeansEqual << ", CI=[" << test.differenceConfidenceIntervalLowerLimit
                     << ", " << test.differenceConfidenceIntervalUpperLimit << "]";
}
}
