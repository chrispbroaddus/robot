#include "packages/benchmarking/include/benchmark.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <thread>
#include <vector>

#include <iostream>

namespace {
using summary_type = SummaryStatistics<double>;
using promise_type = std::promise<std::pair<summary_type, summary_type> >;
using future_type = std::future<std::pair<summary_type, summary_type> >;
using function_type = std::function<int()>;
using clock_type = std::chrono::high_resolution_clock;

// Note: this is a dummy variable. Its only purpose is to make life difficult for the optimizer.
std::atomic<int64_t> dummy;

void benchmarkWorker(promise_type& p, function_type baseline, function_type comparison, size_t outerIterations, size_t innerIterations,
    size_t numThreads, size_t threadIdx) {
    SummaryStatistics<double> baselineStatistics;
    SummaryStatistics<double> comparisonStatistics;

    int64_t countBaseline = 0;
    int64_t countComparison = 0;
    for (size_t o = threadIdx; o < outerIterations; o += numThreads) {
        const auto start = clock_type::now();

        for (size_t i = 0; i < innerIterations; ++i) {
            countBaseline += baseline();
            countBaseline += baseline();
        }

        const auto baselineEnd = clock_type::now();

        for (size_t i = 0; i < innerIterations; ++i) {
            countBaseline += baseline();
            countComparison += comparison();
        }

        const auto comparisonEnd = clock_type::now();

        const double baselineTimePerInnerIteration = std::chrono::duration<double>(baselineEnd - start).count() / (2 * innerIterations);
        baselineStatistics.update(baselineTimePerInnerIteration);

        const double comparisonTimePerInnerIteration
            = std::chrono::duration<double>(comparisonEnd - baselineEnd).count() / innerIterations - baselineTimePerInnerIteration;
        comparisonStatistics.update(comparisonTimePerInnerIteration);
    }

    dummy += countBaseline;
    dummy += countComparison;
    p.set_value(std::make_pair(baselineStatistics, comparisonStatistics));
}
}

BenchmarkResult performBenchmark(
    std::function<int()> baseline, std::function<int()> comparison, size_t outerIterations, size_t innerIterations) {
    const size_t numThreads = std::max<size_t>(1, std::thread::hardware_concurrency());

    std::unique_ptr<std::thread[]> threads(new std::thread[numThreads]);
    std::unique_ptr<future_type[]> futures(new future_type[numThreads]);
    std::unique_ptr<promise_type[]> promises(new promise_type[numThreads]);

    // Scatter out the work
    for (size_t i = 0; i < numThreads; ++i) {
        futures[i] = promises[i].get_future();
        threads[i] = std::thread([i, &promises, baseline, comparison, outerIterations, innerIterations, numThreads]() {
            benchmarkWorker(promises[i], baseline, comparison, outerIterations, innerIterations, numThreads, i);
        });
    }

    // Gather up the results
    summary_type baselineSummary;
    summary_type comparisonSummary;
    for (size_t i = 0; i < numThreads; ++i) {
        threads[i].join();
        const auto x = futures[i].get();
        baselineSummary.merge(x.first);
        comparisonSummary.merge(x.second);
    }

    return BenchmarkResult(baselineSummary, comparisonSummary);
}
