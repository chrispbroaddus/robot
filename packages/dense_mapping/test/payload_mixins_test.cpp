#include "packages/dense_mapping/include/octree_payloads.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <iterator>
#include <random>

namespace {
template <typename T> class OccupiedCountMixinTest : public ::testing::Test {
public:
    template <typename T2> struct Impl : public dense_mapping::payloads::mixins::OccupiedCountMixin<T2, Impl> {
        constexpr Impl()
            : m_occupiedCount(0) {}

        uint64_t m_occupiedCount;
    };

    using ImplementationUnderTest = Impl<T>;
    using Mixin = dense_mapping::payloads::mixins::OccupiedCountMixin<T, Impl>;

    static constexpr std::array<T, 3> kDummy = { { 0, 0, 0 } };
};

template <typename T> constexpr std::array<T, 3> OccupiedCountMixinTest<T>::kDummy;

using TypesToTest = ::testing::Types<float, double>;

TYPED_TEST_CASE(OccupiedCountMixinTest, TypesToTest);

TYPED_TEST(OccupiedCountMixinTest, canInstantiate) { typename TestFixture::ImplementationUnderTest impl; }

TYPED_TEST(OccupiedCountMixinTest, updateIncrements) {
    typename TestFixture::ImplementationUnderTest impl;
    EXPECT_EQ(0, impl.m_occupiedCount);

    TestFixture::Mixin::template updateImpl(impl, TestFixture::kDummy, 0, TestFixture::kDummy, TestFixture::kDummy);
    EXPECT_EQ(1, impl.m_occupiedCount);
}

TYPED_TEST(OccupiedCountMixinTest, mergeEmptyIntoEmpty) {
    using implementation_type = typename TestFixture::ImplementationUnderTest;
    using mixin_type = typename TestFixture::Mixin;

    implementation_type a;
    EXPECT_EQ(0, a.m_occupiedCount);

    implementation_type b;
    EXPECT_EQ(0, b.m_occupiedCount);

    implementation_type c;
    mixin_type::mergeImpl(c, a, b);
    EXPECT_EQ(0, c.m_occupiedCount);
}

TYPED_TEST(OccupiedCountMixinTest, mergeNonemptyIntoEmpty) {
    using implementation_type = typename TestFixture::ImplementationUnderTest;
    using mixin_type = typename TestFixture::Mixin;

    implementation_type a;
    a.m_occupiedCount = 20;

    implementation_type b;
    EXPECT_EQ(0, b.m_occupiedCount);

    implementation_type c;
    mixin_type::mergeImpl(c, a, b);
    EXPECT_EQ(a.m_occupiedCount, c.m_occupiedCount);
}

TYPED_TEST(OccupiedCountMixinTest, mergeEmptyIntoNonempty) {
    using implementation_type = typename TestFixture::ImplementationUnderTest;
    using mixin_type = typename TestFixture::Mixin;

    implementation_type a;
    EXPECT_EQ(0, a.m_occupiedCount);

    implementation_type b;
    b.m_occupiedCount = 20;

    implementation_type c;
    mixin_type::mergeImpl(c, a, b);
    EXPECT_EQ(b.m_occupiedCount, c.m_occupiedCount);
}

TYPED_TEST(OccupiedCountMixinTest, mergeNonemptyIntoNonempty) {
    using implementation_type = typename TestFixture::ImplementationUnderTest;
    using mixin_type = typename TestFixture::Mixin;

    implementation_type a;
    a.m_occupiedCount = 10;

    implementation_type b;
    b.m_occupiedCount = 20;

    implementation_type c;
    mixin_type::mergeImpl(c, a, b);
    EXPECT_EQ(a.m_occupiedCount + b.m_occupiedCount, c.m_occupiedCount);

    implementation_type c2;
    mixin_type::mergeImpl(c2, b, a);
    EXPECT_EQ(a.m_occupiedCount + b.m_occupiedCount, c2.m_occupiedCount);
}
}

namespace {
template <typename T> class SpatialMomentsMixinTest : public ::testing::Test {
public:
    template <typename T2> struct Impl : public dense_mapping::payloads::mixins::SpatialMomentsMixin<T2, Impl> {
        constexpr Impl()
            : m_occupiedCount(0)
            , m_centroidX(0)
            , m_centroidY(0)
            , m_centroidZ(0)
            , m_momentXX(0)
            , m_momentXY(0)
            , m_momentXZ(0)
            , m_momentYY(0)
            , m_momentYZ(0)
            , m_momentZZ(0) {}

        uint64_t m_occupiedCount;
        T2 m_centroidX;
        T2 m_centroidY;
        T2 m_centroidZ;
        T2 m_momentXX;
        T2 m_momentXY;
        T2 m_momentXZ;
        T2 m_momentYY;
        T2 m_momentYZ;
        T2 m_momentZZ;
    };

    using ImplementationUnderTest = Impl<T>;
    using Mixin = dense_mapping::payloads::mixins::SpatialMomentsMixin<T, Impl>;

    static constexpr std::array<T, 3> kDummy = { { 0, 0, 0 } };
};

template <typename T> constexpr std::array<T, 3> SpatialMomentsMixinTest<T>::kDummy;

TYPED_TEST_CASE(SpatialMomentsMixinTest, TypesToTest);

TYPED_TEST(SpatialMomentsMixinTest, canInstantiate) { typename TestFixture::ImplementationUnderTest impl; }

TYPED_TEST(SpatialMomentsMixinTest, saneStateSingleUpdate) {
    typename TestFixture::ImplementationUnderTest impl;
    EXPECT_EQ(0, impl.m_occupiedCount);
    EXPECT_EQ(0, impl.m_centroidX);
    EXPECT_EQ(0, impl.m_centroidY);
    EXPECT_EQ(0, impl.m_centroidZ);
    EXPECT_EQ(0, impl.m_momentXX);
    EXPECT_EQ(0, impl.m_momentXY);
    EXPECT_EQ(0, impl.m_momentXZ);
    EXPECT_EQ(0, impl.m_momentYY);
    EXPECT_EQ(0, impl.m_momentYZ);
    EXPECT_EQ(0, impl.m_momentZZ);

    ++impl.m_occupiedCount;
    TestFixture::Mixin::template updateImpl(impl, TestFixture::kDummy, 0, TestFixture::kDummy, { { 1, 1, 1 } });

    EXPECT_EQ(1, impl.m_occupiedCount);
    EXPECT_EQ(1, impl.m_centroidX);
    EXPECT_EQ(1, impl.m_centroidY);
    EXPECT_EQ(1, impl.m_centroidZ);
    EXPECT_EQ(0, impl.m_momentXX);
    EXPECT_EQ(0, impl.m_momentXY);
    EXPECT_EQ(0, impl.m_momentXZ);
    EXPECT_EQ(0, impl.m_momentYY);
    EXPECT_EQ(0, impl.m_momentYZ);
    EXPECT_EQ(0, impl.m_momentZZ);
}

// https://en.wikipedia.org/wiki/Kahan_summation_algorithm#Further_enhancements
template <typename T> class NeumaierSummer {
public:
    constexpr NeumaierSummer()
        : m_value(0)
        , m_compensation(0) {}

    constexpr operator T() const { return value(); }

    constexpr T value() const { return m_compensation + m_value; }

    NeumaierSummer<T>& operator+=(T item) {
        const auto t = m_value + item;
        m_compensation += (std::abs(item) > std::abs(m_value)) ? ((m_value - t) + item) : ((item - t) + m_value);
        m_value = t;
        return *this;
    }

    NeumaierSummer<T>& operator+=(const NeumaierSummer<T>& item) {
        m_value += item.m_value;
        m_compensation += item.m_compensation;
        return *this;
    }

    NeumaierSummer<T>& operator/=(T item) {
        m_value /= item;
        m_compensation /= item;
        return *this;
    }

private:
    T m_value;
    T m_compensation;
};

template <typename T> struct SpatialMoments {
    constexpr SpatialMoments()
        : m_centroidX()
        , m_centroidY()
        , m_centroidZ()
        , m_momentXX()
        , m_momentXY()
        , m_momentXZ()
        , m_momentYY()
        , m_momentYZ()
        , m_momentZZ() {}

    NeumaierSummer<T> m_centroidX;
    NeumaierSummer<T> m_centroidY;
    NeumaierSummer<T> m_centroidZ;
    NeumaierSummer<T> m_momentXX;
    NeumaierSummer<T> m_momentXY;
    NeumaierSummer<T> m_momentXZ;
    NeumaierSummer<T> m_momentYY;
    NeumaierSummer<T> m_momentYZ;
    NeumaierSummer<T> m_momentZZ;
};

template <typename T> SpatialMoments<T> computeSpatialMoments(const std::vector<std::array<T, 3> >& points) {
    SpatialMoments<T> result;

    for (const auto& p : points) {
        result.m_centroidX += p[0];
        result.m_centroidY += p[1];
        result.m_centroidZ += p[2];
    }

    const auto n = points.empty() ? 1 : points.size();
    result.m_centroidX /= static_cast<T>(n);
    result.m_centroidY /= static_cast<T>(n);
    result.m_centroidZ /= static_cast<T>(n);

    for (const auto& p : points) {
        const auto dx = result.m_centroidX.value() - p[0];
        const auto dy = result.m_centroidY.value() - p[1];
        const auto dz = result.m_centroidZ.value() - p[2];

        result.m_momentXX += dx * dx;
        result.m_momentXY += dx * dy;
        result.m_momentXZ += dx * dz;
        result.m_momentYY += dy * dy;
        result.m_momentYZ += dy * dz;
        result.m_momentZZ += dz * dz;
    }

    return result;
}

template <typename T>
std::vector<std::array<T, 3> > generateCorrelatedSamples(size_t numSamples, std::mt19937::result_type seed, T mean = 0, T stdev = 3) {
    std::mt19937 rng(seed);
    std::normal_distribution<T> dist(mean, stdev);

    // Generate basis vectors initially at random
    std::array<T, 3> basisVectors[3]
        = { { { dist(rng), dist(rng), dist(rng) } }, { { dist(rng), dist(rng), dist(rng) } }, { { dist(rng), dist(rng), dist(rng) } } };

    // Gram-Schmidt to get orthogonal basis -- want this because we want to generate correlated samples
    for (size_t i = 1; i < 3; ++i) {
        basisVectors[i] = dense_mapping::Geometry::unitVector(basisVectors[i]);
        for (size_t j = 0; j < i; ++j) {
            const auto proj = dense_mapping::Geometry::innerProduct(basisVectors[i], basisVectors[j]);
            basisVectors[i][0] -= proj * basisVectors[j][0];
            basisVectors[i][1] -= proj * basisVectors[j][1];
            basisVectors[i][2] -= proj * basisVectors[j][2];
            basisVectors[i] = dense_mapping::Geometry::unitVector(basisVectors[i]);
        }
    }

    // Generate samples
    std::vector<std::array<T, 3> > samples(numSamples);
    std::generate_n(samples.begin(), numSamples, [&]() {
        const std::array<T, 3> pt{ { dist(rng), dist(rng), dist(rng) } };
        return std::array<T, 3>{ { dense_mapping::Geometry::innerProduct(pt, basisVectors[0]),
            dense_mapping::Geometry::innerProduct(pt, basisVectors[1]), dense_mapping::Geometry::innerProduct(pt, basisVectors[2]) } };
    });

    return samples;
}

TYPED_TEST(SpatialMomentsMixinTest, saneStateMultipleUpdates) {
    constexpr std::mt19937::result_type seed = 0;
    constexpr size_t numSamples = 1000;
    const auto samples = generateCorrelatedSamples<TypeParam>(numSamples, seed);
    const auto expected = computeSpatialMoments(samples);

    typename TestFixture::ImplementationUnderTest impl;
    EXPECT_EQ(0, impl.m_occupiedCount);

    for (const auto& pt : samples) {
        ++impl.m_occupiedCount;
        TestFixture::Mixin::template updateImpl(impl, TestFixture::kDummy, 0, TestFixture::kDummy, pt);
    }

    const auto epsilon = 50 * std::numeric_limits<TypeParam>::epsilon();

    EXPECT_EQ(numSamples, impl.m_occupiedCount);
    EXPECT_NEAR(expected.m_centroidX, impl.m_centroidX, epsilon * std::abs(expected.m_centroidX));
    EXPECT_NEAR(expected.m_centroidY, impl.m_centroidY, epsilon * std::abs(expected.m_centroidY));
    EXPECT_NEAR(expected.m_centroidZ, impl.m_centroidZ, epsilon * std::abs(expected.m_centroidZ));
    EXPECT_NEAR(expected.m_momentXX, impl.m_momentXX, epsilon * std::abs(impl.m_momentXX));
    EXPECT_NEAR(expected.m_momentXY, impl.m_momentXY, epsilon * std::abs(impl.m_momentXY));
    EXPECT_NEAR(expected.m_momentXZ, impl.m_momentXZ, epsilon * std::abs(impl.m_momentXZ));
    EXPECT_NEAR(expected.m_momentYY, impl.m_momentYY, epsilon * std::abs(impl.m_momentYY));
    EXPECT_NEAR(expected.m_momentYZ, impl.m_momentYZ, epsilon * std::abs(impl.m_momentYZ));
    EXPECT_NEAR(expected.m_momentZZ, impl.m_momentZZ, epsilon * std::abs(impl.m_momentZZ));
}

TYPED_TEST(SpatialMomentsMixinTest, mergeEmptyIntoEmpty) {
    using implementation_type = typename TestFixture::ImplementationUnderTest;
    using mixin_type = typename TestFixture::Mixin;

    implementation_type a;
    EXPECT_EQ(0, a.m_occupiedCount);
    EXPECT_EQ(0, a.m_centroidX);
    EXPECT_EQ(0, a.m_centroidY);
    EXPECT_EQ(0, a.m_centroidZ);
    EXPECT_EQ(0, a.m_momentXX);
    EXPECT_EQ(0, a.m_momentXY);
    EXPECT_EQ(0, a.m_momentXZ);
    EXPECT_EQ(0, a.m_momentYY);
    EXPECT_EQ(0, a.m_momentYZ);
    EXPECT_EQ(0, a.m_momentZZ);

    implementation_type b;
    EXPECT_EQ(0, b.m_occupiedCount);
    EXPECT_EQ(0, b.m_centroidX);
    EXPECT_EQ(0, b.m_centroidY);
    EXPECT_EQ(0, b.m_centroidZ);
    EXPECT_EQ(0, b.m_momentXX);
    EXPECT_EQ(0, b.m_momentXY);
    EXPECT_EQ(0, b.m_momentXZ);
    EXPECT_EQ(0, b.m_momentYY);
    EXPECT_EQ(0, b.m_momentYZ);
    EXPECT_EQ(0, b.m_momentZZ);

    implementation_type c;
    mixin_type::mergeImpl(c, a, b);
    EXPECT_EQ(0, c.m_occupiedCount);
    EXPECT_EQ(0, c.m_centroidX);
    EXPECT_EQ(0, c.m_centroidY);
    EXPECT_EQ(0, c.m_centroidZ);
    EXPECT_EQ(0, c.m_momentXX);
    EXPECT_EQ(0, c.m_momentXY);
    EXPECT_EQ(0, c.m_momentXZ);
    EXPECT_EQ(0, c.m_momentYY);
    EXPECT_EQ(0, c.m_momentYZ);
    EXPECT_EQ(0, c.m_momentZZ);
}

TYPED_TEST(SpatialMomentsMixinTest, mergeNonemptyIntoEmpty) {
    using implementation_type = typename TestFixture::ImplementationUnderTest;
    using mixin_type = typename TestFixture::Mixin;

    implementation_type a;

    constexpr std::mt19937::result_type seed = 0;
    constexpr size_t numSamples = 1000;
    const auto samples = generateCorrelatedSamples<TypeParam>(numSamples, seed);
    const auto expected = computeSpatialMoments(samples);

    implementation_type b;
    for (const auto& pt : samples) {
        ++b.m_occupiedCount;
        mixin_type::template updateImpl(b, TestFixture::kDummy, 0, TestFixture::kDummy, pt);
    }

    const auto epsilon = 50 * std::numeric_limits<TypeParam>::epsilon();

    EXPECT_EQ(numSamples, b.m_occupiedCount);
    EXPECT_NEAR(expected.m_centroidX, b.m_centroidX, epsilon * std::abs(expected.m_centroidX));
    EXPECT_NEAR(expected.m_centroidY, b.m_centroidY, epsilon * std::abs(expected.m_centroidY));
    EXPECT_NEAR(expected.m_centroidZ, b.m_centroidZ, epsilon * std::abs(expected.m_centroidZ));
    EXPECT_NEAR(expected.m_momentXX, b.m_momentXX, epsilon * std::abs(expected.m_momentXX));
    EXPECT_NEAR(expected.m_momentXY, b.m_momentXY, epsilon * std::abs(expected.m_momentXY));
    EXPECT_NEAR(expected.m_momentXZ, b.m_momentXZ, epsilon * std::abs(expected.m_momentXZ));
    EXPECT_NEAR(expected.m_momentYY, b.m_momentYY, epsilon * std::abs(expected.m_momentYY));
    EXPECT_NEAR(expected.m_momentYZ, b.m_momentYZ, epsilon * std::abs(expected.m_momentYZ));
    EXPECT_NEAR(expected.m_momentZZ, b.m_momentZZ, epsilon * std::abs(expected.m_momentZZ));

    implementation_type c;
    mixin_type::mergeImpl(c, a, b);
    EXPECT_EQ(b.m_centroidX, c.m_centroidX);
    EXPECT_EQ(b.m_centroidY, c.m_centroidY);
    EXPECT_EQ(b.m_centroidZ, c.m_centroidZ);
    EXPECT_EQ(b.m_momentXX, c.m_momentXX);
    EXPECT_EQ(b.m_momentXY, c.m_momentXY);
    EXPECT_EQ(b.m_momentXZ, c.m_momentXZ);
    EXPECT_EQ(b.m_momentYY, c.m_momentYY);
    EXPECT_EQ(b.m_momentYZ, c.m_momentYZ);
    EXPECT_EQ(b.m_momentZZ, c.m_momentZZ);
}

TYPED_TEST(SpatialMomentsMixinTest, mergeEmptyIntoNonempty) {
    using implementation_type = typename TestFixture::ImplementationUnderTest;
    using mixin_type = typename TestFixture::Mixin;

    implementation_type a;

    constexpr std::mt19937::result_type seed = 0;
    constexpr size_t numSamples = 1000;
    const auto samples = generateCorrelatedSamples<TypeParam>(numSamples, seed);
    const auto expected = computeSpatialMoments(samples);

    implementation_type b;
    for (const auto& pt : samples) {
        ++b.m_occupiedCount;
        mixin_type::template updateImpl(b, TestFixture::kDummy, 0, TestFixture::kDummy, pt);
    }

    const auto epsilon = 50 * std::numeric_limits<TypeParam>::epsilon();

    EXPECT_EQ(numSamples, b.m_occupiedCount);
    EXPECT_NEAR(expected.m_centroidX, b.m_centroidX, epsilon * std::abs(expected.m_centroidX));
    EXPECT_NEAR(expected.m_centroidY, b.m_centroidY, epsilon * std::abs(expected.m_centroidY));
    EXPECT_NEAR(expected.m_centroidZ, b.m_centroidZ, epsilon * std::abs(expected.m_centroidZ));
    EXPECT_NEAR(expected.m_momentXX, b.m_momentXX, epsilon * std::abs(expected.m_momentXX));
    EXPECT_NEAR(expected.m_momentXY, b.m_momentXY, epsilon * std::abs(expected.m_momentXY));
    EXPECT_NEAR(expected.m_momentXZ, b.m_momentXZ, epsilon * std::abs(expected.m_momentXZ));
    EXPECT_NEAR(expected.m_momentYY, b.m_momentYY, epsilon * std::abs(expected.m_momentYY));
    EXPECT_NEAR(expected.m_momentYZ, b.m_momentYZ, epsilon * std::abs(expected.m_momentYZ));
    EXPECT_NEAR(expected.m_momentZZ, b.m_momentZZ, epsilon * std::abs(expected.m_momentZZ));

    implementation_type c;
    mixin_type::mergeImpl(c, b, a);
    EXPECT_EQ(b.m_centroidX, c.m_centroidX);
    EXPECT_EQ(b.m_centroidY, c.m_centroidY);
    EXPECT_EQ(b.m_centroidZ, c.m_centroidZ);
    EXPECT_EQ(b.m_momentXX, c.m_momentXX);
    EXPECT_EQ(b.m_momentXY, c.m_momentXY);
    EXPECT_EQ(b.m_momentXZ, c.m_momentXZ);
    EXPECT_EQ(b.m_momentYY, c.m_momentYY);
    EXPECT_EQ(b.m_momentYZ, c.m_momentYZ);
    EXPECT_EQ(b.m_momentZZ, c.m_momentZZ);
}

TYPED_TEST(SpatialMomentsMixinTest, mergeNonemptyIntoNonempty) {
    using implementation_type = typename TestFixture::ImplementationUnderTest;
    using mixin_type = typename TestFixture::Mixin;
    constexpr auto epsilon = 1000 * std::numeric_limits<TypeParam>::epsilon();

    constexpr std::mt19937::result_type seedA = 0;
    constexpr size_t numSamplesA = 1000;
    constexpr TypeParam meanA = 0;
    constexpr TypeParam stdevA = 1;
    const auto samplesA = generateCorrelatedSamples<TypeParam>(numSamplesA, seedA, meanA, stdevA);
    const auto expectedA = computeSpatialMoments(samplesA);

    implementation_type a;
    for (const auto& pt : samplesA) {
        ++a.m_occupiedCount;
        mixin_type::template updateImpl(a, TestFixture::kDummy, 0, TestFixture::kDummy, pt);
    }

    EXPECT_EQ(numSamplesA, a.m_occupiedCount);
    EXPECT_NEAR(expectedA.m_centroidX, a.m_centroidX, epsilon * std::abs(expectedA.m_centroidX));
    EXPECT_NEAR(expectedA.m_centroidY, a.m_centroidY, epsilon * std::abs(expectedA.m_centroidY));
    EXPECT_NEAR(expectedA.m_centroidZ, a.m_centroidZ, epsilon * std::abs(expectedA.m_centroidZ));
    EXPECT_NEAR(expectedA.m_momentXX, a.m_momentXX, epsilon * std::abs(expectedA.m_momentXX));
    EXPECT_NEAR(expectedA.m_momentXY, a.m_momentXY, epsilon * std::abs(expectedA.m_momentXY));
    EXPECT_NEAR(expectedA.m_momentXZ, a.m_momentXZ, epsilon * std::abs(expectedA.m_momentXZ));
    EXPECT_NEAR(expectedA.m_momentYY, a.m_momentYY, epsilon * std::abs(expectedA.m_momentYY));
    EXPECT_NEAR(expectedA.m_momentYZ, a.m_momentYZ, epsilon * std::abs(expectedA.m_momentYZ));
    EXPECT_NEAR(expectedA.m_momentZZ, a.m_momentZZ, epsilon * std::abs(expectedA.m_momentZZ));

    constexpr std::mt19937::result_type seedB = 1;
    constexpr size_t numSamplesB = 8000;
    constexpr TypeParam meanB = 0;
    constexpr TypeParam stdevB = 10;
    const auto samplesB = generateCorrelatedSamples<TypeParam>(numSamplesB, seedB, meanB, stdevB);
    const auto expectedB = computeSpatialMoments(samplesB);

    implementation_type b;
    for (const auto& pt : samplesB) {
        ++b.m_occupiedCount;
        mixin_type::template updateImpl(b, TestFixture::kDummy, 0, TestFixture::kDummy, pt);
    }

    EXPECT_EQ(numSamplesB, b.m_occupiedCount);
    EXPECT_NEAR(expectedB.m_centroidX, b.m_centroidX, epsilon * std::abs(expectedB.m_centroidX));
    EXPECT_NEAR(expectedB.m_centroidY, b.m_centroidY, epsilon * std::abs(expectedB.m_centroidY));
    EXPECT_NEAR(expectedB.m_centroidZ, b.m_centroidZ, epsilon * std::abs(expectedB.m_centroidZ));
    EXPECT_NEAR(expectedB.m_momentXX, b.m_momentXX, epsilon * std::abs(expectedB.m_momentXX));
    EXPECT_NEAR(expectedB.m_momentXY, b.m_momentXY, epsilon * std::abs(expectedB.m_momentXY));
    EXPECT_NEAR(expectedB.m_momentXZ, b.m_momentXZ, epsilon * std::abs(expectedB.m_momentXZ));
    EXPECT_NEAR(expectedB.m_momentYY, b.m_momentYY, epsilon * std::abs(expectedB.m_momentYY));
    EXPECT_NEAR(expectedB.m_momentYZ, b.m_momentYZ, epsilon * std::abs(expectedB.m_momentYZ));
    EXPECT_NEAR(expectedB.m_momentZZ, b.m_momentZZ, epsilon * std::abs(expectedB.m_momentZZ));

    std::vector<std::array<TypeParam, 3> > samplesMerged;
    samplesMerged.reserve(samplesA.size() + samplesB.size());
    std::copy(samplesA.begin(), samplesA.end(), std::back_inserter(samplesMerged));
    std::copy(samplesB.begin(), samplesB.end(), std::back_inserter(samplesMerged));
    const auto expectedMerged = computeSpatialMoments(samplesMerged);

    implementation_type c1;
    mixin_type::mergeImpl(c1, a, b);

    EXPECT_NEAR(expectedMerged.m_centroidX, c1.m_centroidX, epsilon * std::abs(expectedMerged.m_centroidX));
    EXPECT_NEAR(expectedMerged.m_centroidY, c1.m_centroidY, epsilon * std::abs(expectedMerged.m_centroidY));
    EXPECT_NEAR(expectedMerged.m_centroidZ, c1.m_centroidZ, epsilon * std::abs(expectedMerged.m_centroidZ));
    EXPECT_NEAR(expectedMerged.m_momentXX, c1.m_momentXX, epsilon * std::abs(expectedMerged.m_momentXX));
    EXPECT_NEAR(expectedMerged.m_momentXY, c1.m_momentXY, epsilon * std::abs(expectedMerged.m_momentXY));
    EXPECT_NEAR(expectedMerged.m_momentXZ, c1.m_momentXZ, epsilon * std::abs(expectedMerged.m_momentXZ));
    EXPECT_NEAR(expectedMerged.m_momentYY, c1.m_momentYY, epsilon * std::abs(expectedMerged.m_momentYY));
    EXPECT_NEAR(expectedMerged.m_momentYZ, c1.m_momentYZ, epsilon * std::abs(expectedMerged.m_momentYZ));
    EXPECT_NEAR(expectedMerged.m_momentZZ, c1.m_momentZZ, epsilon * std::abs(expectedMerged.m_momentZZ));

    implementation_type c2;
    mixin_type::mergeImpl(c2, b, a);

    EXPECT_NEAR(expectedMerged.m_centroidX, c2.m_centroidX, epsilon * std::abs(expectedMerged.m_centroidX));
    EXPECT_NEAR(expectedMerged.m_centroidY, c2.m_centroidY, epsilon * std::abs(expectedMerged.m_centroidY));
    EXPECT_NEAR(expectedMerged.m_centroidZ, c2.m_centroidZ, epsilon * std::abs(expectedMerged.m_centroidZ));
    EXPECT_NEAR(expectedMerged.m_momentXX, c2.m_momentXX, epsilon * std::abs(expectedMerged.m_momentXX));
    EXPECT_NEAR(expectedMerged.m_momentXY, c2.m_momentXY, epsilon * std::abs(expectedMerged.m_momentXY));
    EXPECT_NEAR(expectedMerged.m_momentXZ, c2.m_momentXZ, epsilon * std::abs(expectedMerged.m_momentXZ));
    EXPECT_NEAR(expectedMerged.m_momentYY, c2.m_momentYY, epsilon * std::abs(expectedMerged.m_momentYY));
    EXPECT_NEAR(expectedMerged.m_momentYZ, c2.m_momentYZ, epsilon * std::abs(expectedMerged.m_momentYZ));
    EXPECT_NEAR(expectedMerged.m_momentZZ, c2.m_momentZZ, epsilon * std::abs(expectedMerged.m_momentZZ));
}

template <typename T> class ViewDirectionMixinTest : public ::testing::Test {
public:
    template <typename T2> struct Impl : public dense_mapping::payloads::mixins::ViewDirectionMixin<T2, Impl> {
        constexpr Impl()
            : m_occupiedCount(0)
            , m_averageViewDirectionX(0)
            , m_averageViewDirectionY(0)
            , m_averageViewDirectionZ(0) {}

        uint64_t m_occupiedCount;
        T2 m_averageViewDirectionX;
        T2 m_averageViewDirectionY;
        T2 m_averageViewDirectionZ;
    };

    using ImplementationUnderTest = Impl<T>;
    using Mixin = dense_mapping::payloads::mixins::ViewDirectionMixin<T, Impl>;

    static constexpr std::array<T, 3> kDummy = { { 0, 0, 0 } };
};

template <typename T> constexpr std::array<T, 3> ViewDirectionMixinTest<T>::kDummy;

TYPED_TEST_CASE(ViewDirectionMixinTest, TypesToTest);

TYPED_TEST(ViewDirectionMixinTest, canInstantiate) { typename TestFixture::ImplementationUnderTest impl; }

TYPED_TEST(ViewDirectionMixinTest, saneStateSingleUpdate) {
    for (int xSign = -1; xSign <= 1; ++xSign) {
        for (int ySign = -1; ySign <= 1; ++ySign) {
            for (int zSign = -1; zSign <= 1; ++zSign) {
                const std::array<TypeParam, 3> source{ { static_cast<TypeParam>(xSign), static_cast<TypeParam>(ySign),
                    static_cast<TypeParam>(zSign) } };

                if (0 != xSign || 0 != ySign || 0 != zSign) {
                    const std::array<TypeParam, 3> expected
                        = dense_mapping::Geometry::normalizedDirectionVector(source, TestFixture::kDummy);
                    typename TestFixture::ImplementationUnderTest impl;

                    ++impl.m_occupiedCount;
                    TestFixture::Mixin::template updateImpl(impl, TestFixture::kDummy, 0, source, TestFixture::kDummy);

                    EXPECT_EQ(1, impl.m_occupiedCount);
                    EXPECT_EQ(expected[0], impl.m_averageViewDirectionX);
                    EXPECT_EQ(expected[1], impl.m_averageViewDirectionY);
                    EXPECT_EQ(expected[2], impl.m_averageViewDirectionZ);
                }
            }
        }
    }
}

TYPED_TEST(ViewDirectionMixinTest, saneStateMultipleUpdates) {
    std::mt19937 rng(0);
    std::normal_distribution<TypeParam> dist(0, 3);

    // Generate samples
    constexpr size_t numSamples = 1000;
    std::vector<std::array<TypeParam, 3> > samples(numSamples);
    std::generate_n(samples.begin(), numSamples, [&]() { return std::array<TypeParam, 3>{ { 10000 + dist(rng), dist(rng), dist(rng) } }; });

    std::vector<std::array<TypeParam, 3> > transformed(numSamples);
    std::transform(samples.begin(), samples.end(), transformed.begin(),
        [&](const auto& x) { return dense_mapping::Geometry::normalizedDirectionVector(x, TestFixture::kDummy); });

    const auto expected = computeSpatialMoments(transformed);

    typename TestFixture::ImplementationUnderTest impl;

    for (const auto& x : samples) {
        ++impl.m_occupiedCount;
        TestFixture::Mixin::template updateImpl(impl, TestFixture::kDummy, 0, x, TestFixture::kDummy);
    }

    const auto epsilon = 20 * std::numeric_limits<TypeParam>::epsilon();

    EXPECT_EQ(numSamples, impl.m_occupiedCount);
    EXPECT_NEAR(expected.m_centroidX, impl.m_averageViewDirectionX, epsilon);
    EXPECT_NEAR(expected.m_centroidY, impl.m_averageViewDirectionY, epsilon);
    EXPECT_NEAR(expected.m_centroidZ, impl.m_averageViewDirectionZ, epsilon);
}

TYPED_TEST(ViewDirectionMixinTest, mergeEmptyIntoEmpty) {
    using implementation_type = typename TestFixture::ImplementationUnderTest;
    using mixin_type = typename TestFixture::Mixin;

    implementation_type a;
    EXPECT_EQ(0, a.m_occupiedCount);
    EXPECT_EQ(0, a.m_averageViewDirectionX);
    EXPECT_EQ(0, a.m_averageViewDirectionY);
    EXPECT_EQ(0, a.m_averageViewDirectionZ);

    implementation_type b;
    EXPECT_EQ(0, b.m_occupiedCount);
    EXPECT_EQ(0, b.m_averageViewDirectionX);
    EXPECT_EQ(0, b.m_averageViewDirectionY);
    EXPECT_EQ(0, b.m_averageViewDirectionZ);

    implementation_type c;
    mixin_type::mergeImpl(c, a, b);
    EXPECT_EQ(0, c.m_occupiedCount);

    EXPECT_EQ(0, c.m_averageViewDirectionX);
    EXPECT_EQ(0, c.m_averageViewDirectionY);
    EXPECT_EQ(0, c.m_averageViewDirectionZ);
}

TYPED_TEST(ViewDirectionMixinTest, mergeNonemptyIntoEmpty) {
    using implementation_type = typename TestFixture::ImplementationUnderTest;
    using mixin_type = typename TestFixture::Mixin;

    implementation_type a;
    EXPECT_EQ(0, a.m_occupiedCount);
    EXPECT_EQ(0, a.m_averageViewDirectionX);
    EXPECT_EQ(0, a.m_averageViewDirectionY);
    EXPECT_EQ(0, a.m_averageViewDirectionZ);

    std::mt19937 rng(0);
    std::normal_distribution<TypeParam> dist(0, 3);

    // Generate samples
    constexpr size_t numSamples = 1000;
    std::vector<std::array<TypeParam, 3> > samples(numSamples);
    std::generate_n(samples.begin(), numSamples, [&]() {
        return std::array<TypeParam, 3>{ { 10000 + dist(rng), dist(rng) - 10000, dist(rng) } };
    });

    std::vector<std::array<TypeParam, 3> > transformed(numSamples);
    std::transform(samples.begin(), samples.end(), transformed.begin(),
        [&](const auto& x) { return dense_mapping::Geometry::normalizedDirectionVector(x, TestFixture::kDummy); });

    const auto expected = computeSpatialMoments(transformed);

    implementation_type b;
    for (const auto& x : samples) {
        ++b.m_occupiedCount;
        TestFixture::Mixin::template updateImpl(b, TestFixture::kDummy, 0, x, TestFixture::kDummy);
    }

    const auto epsilon = 20 * std::numeric_limits<TypeParam>::epsilon();

    EXPECT_EQ(numSamples, b.m_occupiedCount);
    EXPECT_NEAR(expected.m_centroidX, b.m_averageViewDirectionX, epsilon);
    EXPECT_NEAR(expected.m_centroidY, b.m_averageViewDirectionY, epsilon);
    EXPECT_NEAR(expected.m_centroidZ, b.m_averageViewDirectionZ, epsilon);

    implementation_type c;
    mixin_type::mergeImpl(c, a, b);
    EXPECT_EQ(b.m_averageViewDirectionX, c.m_averageViewDirectionX);
    EXPECT_EQ(b.m_averageViewDirectionY, c.m_averageViewDirectionY);
    EXPECT_EQ(b.m_averageViewDirectionZ, c.m_averageViewDirectionZ);
}

TYPED_TEST(ViewDirectionMixinTest, mergeEmptyIntoNonempty) {
    using implementation_type = typename TestFixture::ImplementationUnderTest;
    using mixin_type = typename TestFixture::Mixin;

    implementation_type a;
    EXPECT_EQ(0, a.m_occupiedCount);
    EXPECT_EQ(0, a.m_averageViewDirectionX);
    EXPECT_EQ(0, a.m_averageViewDirectionY);
    EXPECT_EQ(0, a.m_averageViewDirectionZ);

    std::mt19937 rng(0);
    std::normal_distribution<TypeParam> dist(0, 3);

    // Generate samples
    constexpr size_t numSamples = 1000;
    std::vector<std::array<TypeParam, 3> > samples(numSamples);
    std::generate_n(samples.begin(), numSamples, [&]() {
        return std::array<TypeParam, 3>{ { 10000 + dist(rng), dist(rng) - 10000, dist(rng) } };
    });

    std::vector<std::array<TypeParam, 3> > transformed(numSamples);
    std::transform(samples.begin(), samples.end(), transformed.begin(),
        [&](const auto& x) { return dense_mapping::Geometry::normalizedDirectionVector(x, TestFixture::kDummy); });

    const auto expected = computeSpatialMoments(transformed);

    implementation_type b;
    for (const auto& x : samples) {
        ++b.m_occupiedCount;
        TestFixture::Mixin::template updateImpl(b, TestFixture::kDummy, 0, x, TestFixture::kDummy);
    }

    const auto epsilon = 20 * std::numeric_limits<TypeParam>::epsilon();

    EXPECT_EQ(numSamples, b.m_occupiedCount);
    EXPECT_NEAR(expected.m_centroidX, b.m_averageViewDirectionX, epsilon);
    EXPECT_NEAR(expected.m_centroidY, b.m_averageViewDirectionY, epsilon);
    EXPECT_NEAR(expected.m_centroidZ, b.m_averageViewDirectionZ, epsilon);

    implementation_type c;
    mixin_type::mergeImpl(c, b, a);
    EXPECT_EQ(b.m_averageViewDirectionX, c.m_averageViewDirectionX);
    EXPECT_EQ(b.m_averageViewDirectionY, c.m_averageViewDirectionY);
    EXPECT_EQ(b.m_averageViewDirectionZ, c.m_averageViewDirectionZ);
}

TYPED_TEST(ViewDirectionMixinTest, mergeNonemptyIntoNonempty) {
    using implementation_type = typename TestFixture::ImplementationUnderTest;
    using mixin_type = typename TestFixture::Mixin;

    std::mt19937 rng(0);
    std::normal_distribution<TypeParam> dist(0, 3);

    // Generate samples
    constexpr size_t numSamplesA = 1000;
    std::vector<std::array<TypeParam, 3> > samplesA(numSamplesA);
    std::generate_n(samplesA.begin(), numSamplesA, [&]() {
        return std::array<TypeParam, 3>{ { 10000 + dist(rng), dist(rng) - 10000, dist(rng) } };
    });

    std::vector<std::array<TypeParam, 3> > transformedA(numSamplesA);
    std::transform(samplesA.begin(), samplesA.end(), transformedA.begin(),
        [&](const auto& x) { return dense_mapping::Geometry::normalizedDirectionVector(x, TestFixture::kDummy); });

    const auto expectedA = computeSpatialMoments(transformedA);

    implementation_type a;
    for (const auto& x : samplesA) {
        ++a.m_occupiedCount;
        TestFixture::Mixin::template updateImpl(a, TestFixture::kDummy, 0, x, TestFixture::kDummy);
    }

    const auto epsilon = 1000 * std::numeric_limits<TypeParam>::epsilon();

    EXPECT_EQ(numSamplesA, a.m_occupiedCount);
    EXPECT_NEAR(expectedA.m_centroidX, a.m_averageViewDirectionX, epsilon);
    EXPECT_NEAR(expectedA.m_centroidY, a.m_averageViewDirectionY, epsilon);
    EXPECT_NEAR(expectedA.m_centroidZ, a.m_averageViewDirectionZ, epsilon);

    // Generate samples
    constexpr size_t numSamplesB = 4000;
    std::vector<std::array<TypeParam, 3> > samplesB(numSamplesB);
    std::generate_n(samplesB.begin(), numSamplesB, [&]() {
        return std::array<TypeParam, 3>{ { dist(rng) - 10000, dist(rng) + 10000, dist(rng) } };
    });

    std::vector<std::array<TypeParam, 3> > transformedB(numSamplesB);
    std::transform(samplesB.begin(), samplesB.end(), transformedB.begin(),
        [&](const auto& x) { return dense_mapping::Geometry::normalizedDirectionVector(x, TestFixture::kDummy); });

    const auto expectedB = computeSpatialMoments(transformedB);

    implementation_type b;
    for (const auto& x : samplesB) {
        ++b.m_occupiedCount;
        TestFixture::Mixin::template updateImpl(b, TestFixture::kDummy, 0, x, TestFixture::kDummy);
    }

    EXPECT_EQ(numSamplesB, b.m_occupiedCount);
    EXPECT_NEAR(expectedB.m_centroidX, b.m_averageViewDirectionX, epsilon);
    EXPECT_NEAR(expectedB.m_centroidY, b.m_averageViewDirectionY, epsilon);
    EXPECT_NEAR(expectedB.m_centroidZ, b.m_averageViewDirectionZ, epsilon);

    std::vector<std::array<TypeParam, 3> > transformedMerged;
    transformedMerged.reserve(transformedA.size() + transformedB.size());
    std::copy(transformedA.begin(), transformedA.end(), std::back_inserter(transformedMerged));
    std::copy(transformedB.begin(), transformedB.end(), std::back_inserter(transformedMerged));
    const auto expectedMerged = computeSpatialMoments(transformedMerged);

    implementation_type c1;
    mixin_type::mergeImpl(c1, a, b);
    EXPECT_NEAR(expectedMerged.m_centroidX, c1.m_averageViewDirectionX, epsilon);
    EXPECT_NEAR(expectedMerged.m_centroidY, c1.m_averageViewDirectionY, epsilon);
    EXPECT_NEAR(expectedMerged.m_centroidZ, c1.m_averageViewDirectionZ, epsilon);

    implementation_type c2;
    mixin_type::mergeImpl(c2, b, a);
    EXPECT_NEAR(expectedMerged.m_centroidX, c2.m_averageViewDirectionX, epsilon);
    EXPECT_NEAR(expectedMerged.m_centroidY, c2.m_averageViewDirectionY, epsilon);
    EXPECT_NEAR(expectedMerged.m_centroidZ, c2.m_averageViewDirectionZ, epsilon);
}
}