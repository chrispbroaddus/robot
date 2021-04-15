#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>

template <typename T> class SummaryStatistics {
public:
    constexpr SummaryStatistics()
        : mCount(0)
        , mMeanValue(0)
        , mScaledVariance(0)
        , mMaximum(-std::numeric_limits<T>::infinity())
        , mMinimum(std::numeric_limits<T>::infinity()) {}

    /// Merge constructor. Suppose a was computed using samplesA and b was computed using samplesB, then
    /// this instance is equivalent to having streamed
    constexpr SummaryStatistics(const SummaryStatistics& a, const SummaryStatistics& b)
        : mCount(a.mCount + b.mCount)
        , mMeanValue(a.mMeanValue + ((b.mMeanValue - a.mMeanValue) * b.mCount) / (a.mCount + b.mCount))
        , mScaledVariance(a.mScaledVariance + b.mScaledVariance
              + ((b.mMeanValue - a.mMeanValue) * (b.mMeanValue - a.mMeanValue) * a.mCount * b.mCount) / (a.mCount + b.mCount))
        , mMaximum(std::max(a.mMaximum, b.mMaximum))
        , mMinimum(std::min(a.mMinimum, b.mMinimum)) {

        if (a.count() == 0) {
            *this = b;
        } else if (b.count() == 0) {
            *this = a;
        }
    }

    void update(T sample) {
        const auto deltaOld = sample - mMeanValue;
        mMeanValue += deltaOld / ++mCount;
        mScaledVariance += deltaOld * (sample - mMeanValue);
        mMaximum = std::max(mMaximum, sample);
        mMinimum = std::min(mMinimum, sample);
    }

    void clear() {
        mCount = 0;
        mMeanValue = 0;
        mScaledVariance = 0;
        mMaximum = -std::numeric_limits<T>::infinity();
        mMinimum = std::numeric_limits<T>::infinity();
    }

    constexpr T mean() const { return mMeanValue; }

    constexpr T variance() const { return (mCount > 1) ? (mScaledVariance / (mCount - 1)) : 0; }

    constexpr T standardDeviation() const { return std::sqrt(variance()); }

    constexpr uint64_t count() const { return mCount; }

    constexpr T maximum() const { return mMaximum; }

    constexpr T minimum() const { return mMinimum; }

    SummaryStatistics<T>& merge(SummaryStatistics<T> other) {
        *this = SummaryStatistics<T>(*this, other);
        return *this;
    }

private:
    uint64_t mCount;
    T mMeanValue;
    T mScaledVariance;
    T mMaximum;
    T mMinimum;
};

template <typename T> std::ostream& operator<<(std::ostream& out, const SummaryStatistics<T>& summary) {
    std::ios state(nullptr);
    // Remember the current set of format flags, etc.
    state.copyfmt(out);
    const auto f = out.fill();

    constexpr size_t decimalPlaces = 20;
    constexpr size_t totalDigits = 2 * decimalPlaces;
    constexpr size_t precision = decimalPlaces;
    constexpr size_t width = totalDigits + 2; // 1 for . , 1 for leading +/-

    out << std::setfill(' ') << std::fixed << std::setprecision(precision) << "  Count:              [" << std::right << std::setw(width)
        << summary.count() << "]" << std::endl
        << "  Minimum:            [" << std::right << std::setw(width) << summary.minimum() << "]" << std::endl
        << "  Mean:               [" << std::right << std::setw(width) << summary.mean() << "]" << std::endl
        << "  Maximum:            [" << std::right << std::setw(width) << summary.maximum() << "]" << std::endl
        << "  Standard deviation: [" << std::right << std::setw(width) << summary.standardDeviation() << "]" << std::endl;

    // Restore the original set of format flags, etc.
    out.copyfmt(state);
    out.fill(f);
    return out;
}

namespace details {
template <typename T> constexpr T betacf(T a, T b, T x) {
    constexpr int maxIterations = 100;
    T qab = a + b;
    T qap = a + 1;
    T qam = a - 1;
    T c = 1;
    T d = 1 - qab * x / qap;

    if (std::abs(d) < std::numeric_limits<T>::min()) {
        d = std::numeric_limits<T>::min();
    }

    d = 1 / d;
    T h = d;

    for (int m = 1; m <= maxIterations; m++) {
        const T m2 = 2 * m;
        T aa = m * (b - m) * x / ((qam + m2) * (a + m2));
        d = 1 + aa * d;
        if (std::abs(d) < std::numeric_limits<T>::min()) {
            d = std::numeric_limits<T>::min();
        }
        c = 1 + aa / c;
        if (std::abs(c) < std::numeric_limits<T>::min()) {
            c = std::numeric_limits<T>::min();
        }
        d = 1 / d;
        h *= d * c;
        aa = -(a + m) * (qab + m) * x / ((a + m2) * (qap + m2));
        d = 1 + aa * d;
        if (std::abs(d) < std::numeric_limits<T>::min()) {
            d = std::numeric_limits<T>::min();
        }
        c = 1 + aa / c;
        if (std::abs(c) < std::numeric_limits<T>::min()) {
            c = std::numeric_limits<T>::min();
        }
        d = 1 / d;
        const T del = d * c;
        h *= del;

        if (std::abs(del - 1) < std::numeric_limits<T>::epsilon()) {
            break;
        }
    }

    return h;
}

// Taken from http://www.aip.de/groups/soe/local/numres/bookcpdf/c6-4.pdf and copy/pasted. No guarantees about
// goodness / precision / etc.
template <typename T> constexpr T betai(T a, T b, T x) {

    if (x <= 0 || x >= 1) {
        throw std::runtime_error("Invalid x");
    }

    const T bt
        = (x == 0 || x == 1) ? 0 : std::exp(std::lgamma(a + b) - std::lgamma(a) - std::lgamma(b) + a * std::log(x) + b * std::log1p(-x));
    return (x < ((a + 1) / (a + b + 2))) ? bt * betacf(a, b, x) / a : 1 - bt * betacf(b, a, 1 - x) / b;
}

template <typename T> constexpr T cumulativeStudentTDistribution(const T t, const T nu) {
    const T x = nu / (nu + t * t);
    const T a = nu / 2;
    const T b = T(1) / 2;
    return 1 - betai(a, b, x);
}
}

template <typename T> struct WelchTTestResult {
    constexpr WelchTTestResult(T tStat, T deg, T prob, T low, T hi)
        : tStatistic(tStat)
        , effectiveDegreesOfFreedom(deg)
        , probabilityMeansEqual(prob)
        , differenceConfidenceIntervalLowerLimit(low)
        , differenceConfidenceIntervalUpperLimit(hi) {}

    const T tStatistic;
    const T effectiveDegreesOfFreedom;
    const T probabilityMeansEqual;

    // Note: computed at 95% confidence assuming effectively infinite degrees of freedom
    const T differenceConfidenceIntervalLowerLimit;
    const T differenceConfidenceIntervalUpperLimit;
};

/// https://en.wikipedia.org/wiki/Welch%27s_t-test
template <typename T> WelchTTestResult<T> performWelchTest(const SummaryStatistics<T>& a, const SummaryStatistics<T>& b) {
    const auto pooledVariance = a.variance() / a.count() + b.variance() / b.count();

    const auto tStat = (a.mean() - b.mean()) / std::sqrt(pooledVariance);
    const auto nu1 = a.count() - 1;
    const auto nu2 = b.count() - 1;
    const auto nuNumer = pooledVariance * pooledVariance;
    const auto nuDen1 = (a.variance() * a.variance()) / (a.count() * a.count() * nu1);
    const auto nuDen2 = (b.variance() * b.variance()) / (b.count() * b.count() * nu2);
    const auto nuEffective = std::ceil(nuNumer / (nuDen1 + nuDen2));
    const auto upperLimit = tStat > 0 ? tStat : -tStat;
    const auto probability = 1 - details::cumulativeStudentTDistribution(-upperLimit, nuEffective);
    constexpr T tCutoff = T(1.645);
    const auto differenceFactor = std::sqrt(a.variance() + b.variance());
    const auto lowerCI = (a.mean() - b.mean()) - tCutoff * differenceFactor;
    const auto upperCI = (a.mean() - b.mean()) + tCutoff * differenceFactor;
    return WelchTTestResult<T>(tStat, nuEffective, probability, lowerCI, upperCI);
}

template <typename T> std::ostream& operator<<(std::ostream& out, const WelchTTestResult<T>& test) {
    std::ios state(nullptr);
    // Remember the current set of format flags, etc.
    state.copyfmt(out);
    const auto f = out.fill();

    constexpr size_t decimalPlaces = 20;
    constexpr size_t totalDigits = 2 * decimalPlaces;
    constexpr size_t precision = decimalPlaces;
    constexpr size_t width = totalDigits + 2; // 1 for . , 1 for leading +/-

    out << "Welch T-Test" << std::fixed << std::setprecision(precision) << std::endl
        << "  T-statistic:                   " << std::right << std::setw(width) << test.tStatistic << std::endl
        << "  Effective d.o.f.:              " << std::right << std::setw(width) << test.effectiveDegreesOfFreedom << std::endl
        << "  Pr[equal means]:               " << std::right << std::setw(width) << test.probabilityMeansEqual << std::endl
        << "  Difference (95% @ +Inf d.o.f.) " << std::right << std::setw(width) << test.differenceConfidenceIntervalLowerLimit << " -> "
        << std::right << std::setw(width) << test.differenceConfidenceIntervalUpperLimit << std::endl;

    // Restore the original set of format flags, etc.
    out.copyfmt(state);
    out.fill(f);
    return out;
}
