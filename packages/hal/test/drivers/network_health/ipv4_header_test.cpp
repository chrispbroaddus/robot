#include "packages/hal/include/drivers/network_health/ipv4_header.h"
#include "gtest/gtest.h"

#include <vector>

TEST(IPV4HeaderTest, defaultConstructor) {
    hal::details::IPV4Header header;
    EXPECT_EQ(0, header.version());
    EXPECT_EQ(0, header.internetHeaderLength());
    EXPECT_EQ(0, header.differentiatedServiceCodePoint());
    EXPECT_EQ(0, header.explicitCongestionNotification());
    EXPECT_EQ(0, header.packetLength());
    EXPECT_EQ(0, header.identification());
    EXPECT_EQ(false, header.dontFragment());
    EXPECT_EQ(false, header.moreFragments());
    EXPECT_EQ(0, header.fragmentOffset());
    EXPECT_EQ(0, header.timeToLive());
    EXPECT_EQ(0, header.protocol());
    EXPECT_EQ(0, header.sourceAddress().to_ulong());
    EXPECT_EQ(0, header.destinationAddress().to_ulong());
}

TEST(IPV4HeaderTest, throwsOnEmptyRange) {
    std::vector<uint8_t> src;
    EXPECT_THROW(hal::details::IPV4Header::fromIterator(src.begin(), src.end()), std::invalid_argument);
}

TEST(IPV4HeaderTest, throwsOnTooSmallRange) {
    std::vector<uint8_t> src(19, 0);
    EXPECT_THROW(hal::details::IPV4Header::fromIterator(src.begin(), src.end()), std::invalid_argument);
}

TEST(IPV4HeaderTest, throwsOnTooLargeRange) {
    std::vector<uint8_t> src(61, 0);
    EXPECT_THROW(hal::details::IPV4Header::fromIterator(src.begin(), src.end()), std::invalid_argument);
}

TEST(IPV4HeaderTest, correctlyParsesVersion) {
    std::vector<uint8_t> src(20, 0);
    src[0] = 0x40;

    const auto header = hal::details::IPV4Header::fromIterator(src.begin(), src.end());
    EXPECT_EQ(4, header.version());
    EXPECT_EQ(0, header.internetHeaderLength());
    EXPECT_EQ(0, header.differentiatedServiceCodePoint());
    EXPECT_EQ(0, header.explicitCongestionNotification());
    EXPECT_EQ(0, header.packetLength());
    EXPECT_EQ(0, header.identification());
    EXPECT_EQ(false, header.dontFragment());
    EXPECT_EQ(false, header.moreFragments());
    EXPECT_EQ(0, header.fragmentOffset());
    EXPECT_EQ(0, header.timeToLive());
    EXPECT_EQ(0, header.protocol());
    EXPECT_EQ(0, header.sourceAddress().to_ulong());
    EXPECT_EQ(0, header.destinationAddress().to_ulong());
}

TEST(IPV4HeaderTest, correctlyParsesHeaderLength) {
    std::vector<uint8_t> src(20, 0);

    for (size_t i = 5; i < 16; ++i) {
        src[0] = i;

        const auto header = hal::details::IPV4Header::fromIterator(src.begin(), src.end());
        EXPECT_EQ(0, header.version());

        EXPECT_EQ(i * 4, header.internetHeaderLength());
        EXPECT_EQ(0, header.differentiatedServiceCodePoint());
        EXPECT_EQ(0, header.explicitCongestionNotification());
        EXPECT_EQ(0, header.packetLength());
        EXPECT_EQ(0, header.identification());
        EXPECT_EQ(false, header.dontFragment());
        EXPECT_EQ(false, header.moreFragments());
        EXPECT_EQ(0, header.fragmentOffset());
        EXPECT_EQ(0, header.timeToLive());
        EXPECT_EQ(0, header.protocol());
        EXPECT_EQ(0, header.sourceAddress().to_ulong());
        EXPECT_EQ(0, header.destinationAddress().to_ulong());
    }
}

TEST(IPV4HeaderTest, correctlyParsesDCSP) {
    std::vector<uint8_t> src(20, 0);

    for (size_t i = 0; i < 64; ++i) {
        src[1] = (i << 2) & (~0x3);

        const auto header = hal::details::IPV4Header::fromIterator(src.begin(), src.end());
        EXPECT_EQ(0, header.version());

        EXPECT_EQ(0, header.internetHeaderLength());
        EXPECT_EQ(i, header.differentiatedServiceCodePoint());
        EXPECT_EQ(0, header.explicitCongestionNotification());
        EXPECT_EQ(0, header.packetLength());
        EXPECT_EQ(0, header.identification());
        EXPECT_EQ(false, header.dontFragment());
        EXPECT_EQ(false, header.moreFragments());
        EXPECT_EQ(0, header.fragmentOffset());
        EXPECT_EQ(0, header.timeToLive());
        EXPECT_EQ(0, header.protocol());
        EXPECT_EQ(0, header.sourceAddress().to_ulong());
        EXPECT_EQ(0, header.destinationAddress().to_ulong());
    }
}

TEST(IPV4HeaderTest, correctlyParsesECN) {
    std::vector<uint8_t> src(20, 0);

    for (size_t i = 0; i < 4; ++i) {
        src[1] = i;

        const auto header = hal::details::IPV4Header::fromIterator(src.begin(), src.end());
        EXPECT_EQ(0, header.version());

        EXPECT_EQ(0, header.internetHeaderLength());
        EXPECT_EQ(0, header.differentiatedServiceCodePoint());
        EXPECT_EQ(i, header.explicitCongestionNotification());
        EXPECT_EQ(0, header.packetLength());
        EXPECT_EQ(0, header.identification());
        EXPECT_EQ(false, header.dontFragment());
        EXPECT_EQ(false, header.moreFragments());
        EXPECT_EQ(0, header.fragmentOffset());
        EXPECT_EQ(0, header.timeToLive());
        EXPECT_EQ(0, header.protocol());
        EXPECT_EQ(0, header.sourceAddress().to_ulong());
        EXPECT_EQ(0, header.destinationAddress().to_ulong());
    }
}

TEST(IPV4HeaderTest, correctlyParsesTotalLength) {
    std::vector<uint8_t> src(20, 0);

    for (size_t i = 0; i < 0x10000; ++i) {
        src[2] = (i >> 8) & 0xff;
        src[3] = i & 0xff;

        const auto header = hal::details::IPV4Header::fromIterator(src.begin(), src.end());
        EXPECT_EQ(0, header.version());

        EXPECT_EQ(0, header.internetHeaderLength());
        EXPECT_EQ(0, header.differentiatedServiceCodePoint());
        EXPECT_EQ(0, header.explicitCongestionNotification());
        EXPECT_EQ(i, header.packetLength());
        EXPECT_EQ(0, header.identification());
        EXPECT_EQ(false, header.dontFragment());
        EXPECT_EQ(false, header.moreFragments());
        EXPECT_EQ(0, header.fragmentOffset());
        EXPECT_EQ(0, header.timeToLive());
        EXPECT_EQ(0, header.protocol());
        EXPECT_EQ(0, header.sourceAddress().to_ulong());
        EXPECT_EQ(0, header.destinationAddress().to_ulong());
    }
}

TEST(IPV4HeaderTest, correctlyParsesIdentification) {
    std::vector<uint8_t> src(20, 0);

    for (size_t i = 0; i < 0x10000; ++i) {
        src[4] = (i >> 8) & 0xff;
        src[5] = i & 0xff;

        const auto header = hal::details::IPV4Header::fromIterator(src.begin(), src.end());
        EXPECT_EQ(0, header.version());

        EXPECT_EQ(0, header.internetHeaderLength());
        EXPECT_EQ(0, header.differentiatedServiceCodePoint());
        EXPECT_EQ(0, header.explicitCongestionNotification());
        EXPECT_EQ(0, header.packetLength());
        EXPECT_EQ(i, header.identification());
        EXPECT_EQ(false, header.dontFragment());
        EXPECT_EQ(false, header.moreFragments());
        EXPECT_EQ(0, header.fragmentOffset());
        EXPECT_EQ(0, header.timeToLive());
        EXPECT_EQ(0, header.protocol());
        EXPECT_EQ(0, header.sourceAddress().to_ulong());
        EXPECT_EQ(0, header.destinationAddress().to_ulong());
    }
}

TEST(IPV4HeaderTest, correctlyParsesDontFragment) {
    std::vector<uint8_t> src(20, 0);
    src[6] = 0x40;

    const auto header = hal::details::IPV4Header::fromIterator(src.begin(), src.end());
    EXPECT_EQ(0, header.version());
    EXPECT_EQ(0, header.internetHeaderLength());
    EXPECT_EQ(0, header.differentiatedServiceCodePoint());
    EXPECT_EQ(0, header.explicitCongestionNotification());
    EXPECT_EQ(0, header.packetLength());
    EXPECT_EQ(0, header.identification());
    EXPECT_EQ(true, header.dontFragment());
    EXPECT_EQ(false, header.moreFragments());
    EXPECT_EQ(0, header.fragmentOffset());
    EXPECT_EQ(0, header.timeToLive());
    EXPECT_EQ(0, header.protocol());
    EXPECT_EQ(0, header.sourceAddress().to_ulong());
    EXPECT_EQ(0, header.destinationAddress().to_ulong());
}

TEST(IPV4HeaderTest, correctlyParsesMoreFragments) {
    std::vector<uint8_t> src(20, 0);
    src[6] = 0x20;

    const auto header = hal::details::IPV4Header::fromIterator(src.begin(), src.end());
    EXPECT_EQ(0, header.version());
    EXPECT_EQ(0, header.internetHeaderLength());
    EXPECT_EQ(0, header.differentiatedServiceCodePoint());
    EXPECT_EQ(0, header.explicitCongestionNotification());
    EXPECT_EQ(0, header.packetLength());
    EXPECT_EQ(0, header.identification());
    EXPECT_EQ(false, header.dontFragment());
    EXPECT_EQ(true, header.moreFragments());
    EXPECT_EQ(0, header.fragmentOffset());
    EXPECT_EQ(0, header.timeToLive());
    EXPECT_EQ(0, header.protocol());
    EXPECT_EQ(0, header.sourceAddress().to_ulong());
    EXPECT_EQ(0, header.destinationAddress().to_ulong());
}

TEST(IPV4HeaderTest, correctlyParsesFragmentOffset) {
    std::vector<uint8_t> src(20, 0);

    for (size_t i = 0; i < 0x2000; ++i) {
        src[6] = (i >> 8) & 0xff;
        src[7] = i & 0xff;

        const auto header = hal::details::IPV4Header::fromIterator(src.begin(), src.end());
        EXPECT_EQ(0, header.version());

        EXPECT_EQ(0, header.internetHeaderLength());
        EXPECT_EQ(0, header.differentiatedServiceCodePoint());
        EXPECT_EQ(0, header.explicitCongestionNotification());
        EXPECT_EQ(0, header.packetLength());
        EXPECT_EQ(0, header.identification());
        EXPECT_EQ(false, header.dontFragment());
        EXPECT_EQ(false, header.moreFragments());
        EXPECT_EQ(i, header.fragmentOffset());
        EXPECT_EQ(0, header.timeToLive());
        EXPECT_EQ(0, header.protocol());
        EXPECT_EQ(0, header.sourceAddress().to_ulong());
        EXPECT_EQ(0, header.destinationAddress().to_ulong());
    }
}

TEST(IPV4HeaderTest, correctlyParsesTTL) {
    std::vector<uint8_t> src(20, 0);

    for (size_t i = 0; i < 0x100; ++i) {
        src[8] = i;

        const auto header = hal::details::IPV4Header::fromIterator(src.begin(), src.end());
        EXPECT_EQ(0, header.version());

        EXPECT_EQ(0, header.internetHeaderLength());
        EXPECT_EQ(0, header.differentiatedServiceCodePoint());
        EXPECT_EQ(0, header.explicitCongestionNotification());
        EXPECT_EQ(0, header.packetLength());
        EXPECT_EQ(0, header.identification());
        EXPECT_EQ(false, header.dontFragment());
        EXPECT_EQ(false, header.moreFragments());
        EXPECT_EQ(0, header.fragmentOffset());
        EXPECT_EQ(i, header.timeToLive());
        EXPECT_EQ(0, header.protocol());
        EXPECT_EQ(0, header.sourceAddress().to_ulong());
        EXPECT_EQ(0, header.destinationAddress().to_ulong());
    }
}

TEST(IPV4HeaderTest, correctlyParsesProtocol) {
    std::vector<uint8_t> src(20, 0);

    for (size_t i = 0; i < 0x100; ++i) {
        src[9] = i;

        const auto header = hal::details::IPV4Header::fromIterator(src.begin(), src.end());
        EXPECT_EQ(0, header.version());

        EXPECT_EQ(0, header.internetHeaderLength());
        EXPECT_EQ(0, header.differentiatedServiceCodePoint());
        EXPECT_EQ(0, header.explicitCongestionNotification());
        EXPECT_EQ(0, header.packetLength());
        EXPECT_EQ(0, header.identification());
        EXPECT_EQ(false, header.dontFragment());
        EXPECT_EQ(false, header.moreFragments());
        EXPECT_EQ(0, header.fragmentOffset());
        EXPECT_EQ(0, header.timeToLive());
        EXPECT_EQ(i, header.protocol());
        EXPECT_EQ(0, header.sourceAddress().to_ulong());
        EXPECT_EQ(0, header.destinationAddress().to_ulong());
    }
}

TEST(IPV4HeaderTest, correctlyParsesSourceIpAddress) {

    std::array<std::array<uint8_t, 4>, 4> expected{ {
        { { 0xff, 0x00, 0x00, 0x00 } }, { { 0x00, 0xff, 0x00, 0x00 } }, { { 0x00, 0x00, 0xff, 0x00 } }, { { 0x00, 0x00, 0x00, 0xff } },
    } };

    for (size_t i = 0; i < 4; ++i) {
        std::vector<uint8_t> src(20, 0);
        src[12 + i] = 0xff;

        const auto header = hal::details::IPV4Header::fromIterator(src.begin(), src.end());
        EXPECT_EQ(0, header.version());
        EXPECT_EQ(0, header.internetHeaderLength());
        EXPECT_EQ(0, header.differentiatedServiceCodePoint());
        EXPECT_EQ(0, header.explicitCongestionNotification());
        EXPECT_EQ(0, header.packetLength());
        EXPECT_EQ(0, header.identification());
        EXPECT_EQ(false, header.dontFragment());
        EXPECT_EQ(false, header.moreFragments());
        EXPECT_EQ(0, header.fragmentOffset());
        EXPECT_EQ(0, header.timeToLive());
        EXPECT_EQ(0, header.protocol());
        EXPECT_EQ(expected.at(i), header.sourceAddress().to_bytes());
        EXPECT_EQ(0, header.destinationAddress().to_ulong());
    }
}

TEST(IPV4HeaderTest, correctlyParsesDestinationIpAddress) {

    std::array<std::array<uint8_t, 4>, 4> expected{ {
        { { 0xff, 0x00, 0x00, 0x00 } }, { { 0x00, 0xff, 0x00, 0x00 } }, { { 0x00, 0x00, 0xff, 0x00 } }, { { 0x00, 0x00, 0x00, 0xff } },
    } };

    for (size_t i = 0; i < 4; ++i) {
        std::vector<uint8_t> src(20, 0);
        src[16 + i] = 0xff;

        const auto header = hal::details::IPV4Header::fromIterator(src.begin(), src.end());
        EXPECT_EQ(0, header.version());
        EXPECT_EQ(0, header.internetHeaderLength());
        EXPECT_EQ(0, header.differentiatedServiceCodePoint());
        EXPECT_EQ(0, header.explicitCongestionNotification());
        EXPECT_EQ(0, header.packetLength());
        EXPECT_EQ(0, header.identification());
        EXPECT_EQ(false, header.dontFragment());
        EXPECT_EQ(false, header.moreFragments());
        EXPECT_EQ(0, header.fragmentOffset());
        EXPECT_EQ(0, header.timeToLive());
        EXPECT_EQ(0, header.protocol());
        EXPECT_EQ(0, header.sourceAddress().to_ulong());
        EXPECT_EQ(expected.at(i), header.destinationAddress().to_bytes());
    }
}