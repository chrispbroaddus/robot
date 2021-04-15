#pragma once

#include "summary_statistics.h"
#include <functional>

/// Result of performing a benchmark.
struct BenchmarkResult {
    BenchmarkResult(SummaryStatistics<double> baseline, SummaryStatistics<double> comparison)
        : baselineOuterLoopStatisticsSecondsPerIteration(baseline)
        , comparisonOuterLoopStatisticsSecondsPerIteration(comparison)
        , analysis(performWelchTest(baselineOuterLoopStatisticsSecondsPerIteration, comparisonOuterLoopStatisticsSecondsPerIteration)) {}

    constexpr BenchmarkResult(
        SummaryStatistics<double> baseline, SummaryStatistics<double> comparison, WelchTTestResult<double> tTestResult)
        : baselineOuterLoopStatisticsSecondsPerIteration(baseline)
        , comparisonOuterLoopStatisticsSecondsPerIteration(comparison)
        , analysis(tTestResult) {}

    /// Summary statistics for the baseline option
    const SummaryStatistics<double> baselineOuterLoopStatisticsSecondsPerIteration;

    /// Summary statistics for the comparison option
    const SummaryStatistics<double> comparisonOuterLoopStatisticsSecondsPerIteration;

    /// Analysis of the differences in the summary statistics. The key pieces here are the estimated confidence
    /// interval on the difference of means as well as the likelihood that we would see a larger t-statistic
    /// by chance alone (i.e. can we reject the null hypothesis that the means are equal)
    const WelchTTestResult<double> analysis;
};

/// Perform a benchmark. This is a somewhat complicated process as we are trying to ensure that we correctly account
/// for things like hyperthreading, etc. Important things to be aware of:
///
/// -# This is threaded. We use \emph{all} cores (as determined by \pre{std::thread::hardware_concurrency()}) concurrently.
///    As a result, we will need to do some refactoring to make this appropriate to measure threaded code.
/// -# We make a 3 calls to the baseline per inner iteration. This follows Andrei Alexandrescu's advice: two of these
///    measure just the baseline. One of these is used to recreate the same conditions in terms of cache thrashing, etc.
///    as we go to measure the comparison option. If your baseline is really slow, you may want to consider swapping
///    the baseline and comparison.
/// -# We only time a complete set of inner iterations. It is important that innerIterations be set large enough that
///    we can get reasonably consistent timings with the high resolution clock, so aim to make an inner iteration take, e.g.,
///    milliseconds at least for both.
/// -# Outer iterations should be at least the number of cores and probably something like 30+ per core to get sensible,
///    reasonable timings.
/// -# Benchmark functions need to return some function of the thing they're trying to measure. This is to ensure that
///    we stop the optimizer being smart and optimizing away all the stuff that we actually care about. These results are
///    used in a way that the compiler can't throw away which hopefully means that what you see is what you get.
/// .
/// \param baseline
/// \param comparison
/// \param outerIterations
/// \param innerIterations
/// \return
BenchmarkResult performBenchmark(
    std::function<int()> baseline, std::function<int()> comparison, size_t outerIterations, size_t innerIterations);
