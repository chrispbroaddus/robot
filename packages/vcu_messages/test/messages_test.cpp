#include "packages/vcu_messages/include/messages.h"
#include "gtest/gtest.h"
#include <numeric>

namespace {
constexpr vcu::EmbeddedUnitIdentifier targets[] = { vcu::EmbeddedUnitIdentifier::LeftFrontMotorController,
    vcu::EmbeddedUnitIdentifier::LeftRearMotorController, vcu::EmbeddedUnitIdentifier::RightFrontMotorController,
    vcu::EmbeddedUnitIdentifier::RightRearMotorController, vcu::EmbeddedUnitIdentifier::VehicleController };
}

/// @note
///
/// If you're modifying these tests, there are some things you should be aware of:
/// 1. All the encode tests should always be implemented with an independent decoding step to verify the contents
///    of the generated buffer.
/// 2. Because all the encode tests have independent decoding, all the decode tests can "just" verify idempotence, that
///    is ensure that X == decode(encode(X)).

TEST(MessagesTest, encode_headerOnlyMessageLowLevelZeroBuffer) {
    constexpr vcu::MessageType messages[] = { vcu::MessageType::DescribeConfiguration, vcu::MessageType::GetConfigurationImage,
        vcu::MessageType::DescribeFirmware, vcu::MessageType::GetFirmwareImage, vcu::MessageType::StartedBootingImage };

    std::vector<uint8_t> buffer;
    for (auto m : messages) {
        for (auto t : targets) {
            EXPECT_EQ(vcu::CodecStatus::BadBufferSize, vcu::encode(m, t, buffer.begin(), buffer.end()));
        }
    }
}

TEST(MessagesTest, encode_headerOnlyMessageLowLevelSmallBuffer) {
    constexpr vcu::MessageType messages[] = { vcu::MessageType::DescribeConfiguration, vcu::MessageType::GetConfigurationImage,
        vcu::MessageType::DescribeFirmware, vcu::MessageType::GetFirmwareImage, vcu::MessageType::StartedBootingImage };

    std::vector<uint8_t> buffer(5, 0);
    for (auto m : messages) {
        for (auto t : targets) {
            EXPECT_EQ(vcu::CodecStatus::BadBufferSize, vcu::encode(m, t, buffer.begin(), buffer.end()));
        }
    }
}

TEST(MessagesTest, encode_headerOnlyMessageLowLevelHappyPath) {
    constexpr vcu::MessageType messages[] = { vcu::MessageType::DescribeConfiguration, vcu::MessageType::GetConfigurationImage,
        vcu::MessageType::DescribeFirmware, vcu::MessageType::GetFirmwareImage, vcu::MessageType::StartedBootingImage };

    constexpr uint16_t expectedSize = 6;
    for (auto m : messages) {
        for (auto t : targets) {
            std::vector<uint8_t> buffer(6);
            EXPECT_EQ(vcu::CodecStatus::Success, vcu::encode(m, t, buffer.begin(), buffer.end()));
            const uint16_t msgSize = (buffer.at(0) << 8) | buffer.at(1);
            EXPECT_EQ(expectedSize, msgSize);
            const uint16_t rawType = (buffer.at(2) << 8) | buffer.at(3);
            EXPECT_EQ(static_cast<std::underlying_type<vcu::MessageType>::type>(m), rawType);
            const uint16_t rawTarget = (buffer.at(4) << 8) | buffer.at(5);
            EXPECT_EQ(static_cast<std::underlying_type<vcu::EmbeddedUnitIdentifier>::type>(t), rawTarget);
        }
    }
}

TEST(MessagesTest, encode_headerOnlyMessageHighLevelHappyPath) {
    constexpr vcu::MessageType messages[] = { vcu::MessageType::DescribeConfiguration, vcu::MessageType::GetConfigurationImage,
        vcu::MessageType::DescribeFirmware, vcu::MessageType::GetFirmwareImage, vcu::MessageType::StartedBootingImage };

    constexpr uint16_t expectedSize = 6;
    for (auto m : messages) {
        for (auto t : targets) {
            vcu::Message msg;
            msg.type = m;
            msg.target = t;
            // deliberately huge buffer
            std::vector<uint8_t> buffer(20, 0);
            EXPECT_EQ(vcu::CodecStatus::Success, vcu::encode(msg, buffer.begin(), buffer.end()));
            const uint16_t msgSize = (buffer.at(0) << 8) | buffer.at(1);
            EXPECT_EQ(expectedSize, msgSize);
            const uint16_t rawType = (buffer.at(2) << 8) | buffer.at(3);
            EXPECT_EQ(static_cast<std::underlying_type<vcu::MessageType>::type>(m), rawType);
            const uint16_t rawTarget = (buffer.at(4) << 8) | buffer.at(5);
            EXPECT_EQ(static_cast<std::underlying_type<vcu::EmbeddedUnitIdentifier>::type>(t), rawTarget);
        }
    }
}

TEST(MessagesTest, encode_versionResponsePayloadLowLevelZeroBuffer) {
    constexpr vcu::MessageType messages[]
        = { vcu::MessageType::DescribeConfigurationResponse, vcu::MessageType::DescribeConfigurationResponse };

    vcu::VersionResponsePayload expectedPayload;
    expectedPayload.sizeBytes = 0xfedcba9876543210LLU;
    std::iota(expectedPayload.checksumSha256.rbegin(), expectedPayload.checksumSha256.rend(), 0);

    std::vector<uint8_t> buffer;
    for (auto m : messages) {
        for (auto t : targets) {
            const auto payload = expectedPayload;
            EXPECT_EQ(vcu::CodecStatus::BadBufferSize, vcu::encode(m, t, payload, buffer.begin(), buffer.end()));
        }
    }
}

TEST(MessagesTest, encode_versionResponsePayloadLowLevelSmallBuffer) {
    constexpr vcu::MessageType messages[]
        = { vcu::MessageType::DescribeConfigurationResponse, vcu::MessageType::DescribeConfigurationResponse };

    vcu::VersionResponsePayload expectedPayload;
    expectedPayload.sizeBytes = 0xfedcba9876543210LLU;
    std::iota(expectedPayload.checksumSha256.rbegin(), expectedPayload.checksumSha256.rend(), 0);

    // minimum size is 6 + 8 + 32 = 46
    std::vector<uint8_t> buffer(45, 0);
    for (auto m : messages) {
        for (auto t : targets) {
            const auto payload = expectedPayload;
            EXPECT_EQ(vcu::CodecStatus::BadBufferSize, vcu::encode(m, t, payload, buffer.begin(), buffer.end()));
        }
    }
}

TEST(MessagesTest, encode_versionResponsePayloadLowLevelHappyPath) {
    constexpr vcu::MessageType messages[]
        = { vcu::MessageType::DescribeConfigurationResponse, vcu::MessageType::DescribeConfigurationResponse };

    vcu::VersionResponsePayload expectedPayload;
    expectedPayload.sizeBytes = 0xfedcba9876543210LLU;
    std::iota(expectedPayload.checksumSha256.rbegin(), expectedPayload.checksumSha256.rend(), 0);

    // minimum size is 6 + 8 + 32 = 46
    constexpr uint16_t expectedSize = 46;
    for (auto m : messages) {
        for (auto t : targets) {
            std::vector<uint8_t> buffer(expectedSize, 0);

            const auto payload = expectedPayload;
            EXPECT_EQ(vcu::CodecStatus::Success, vcu::encode(m, t, payload, buffer.begin(), buffer.end()));

            const uint16_t msgSize = (buffer.at(0) << 8) | buffer.at(1);
            EXPECT_EQ(expectedSize, msgSize);
            const uint16_t rawType = (buffer.at(2) << 8) | buffer.at(3);
            EXPECT_EQ(static_cast<std::underlying_type<vcu::MessageType>::type>(m), rawType);
            const uint16_t rawTarget = (buffer.at(4) << 8) | buffer.at(5);
            EXPECT_EQ(static_cast<std::underlying_type<vcu::EmbeddedUnitIdentifier>::type>(t), rawTarget);

            const uint64_t imageSize = (static_cast<uint64_t>(buffer.at(6)) << 56) | (static_cast<uint64_t>(buffer.at(7)) << 48)
                | (static_cast<uint64_t>(buffer.at(8)) << 40) | (static_cast<uint64_t>(buffer.at(9)) << 32)
                | (static_cast<uint64_t>(buffer.at(10)) << 24) | (static_cast<uint64_t>(buffer.at(11)) << 16)
                | (static_cast<uint64_t>(buffer.at(12)) << 8) | static_cast<uint64_t>(buffer.at(13));

            EXPECT_EQ(expectedPayload.sizeBytes, imageSize);

            EXPECT_EQ(31, buffer.at(14));
            EXPECT_EQ(30, buffer.at(15));
            EXPECT_EQ(29, buffer.at(16));
            EXPECT_EQ(28, buffer.at(17));
            EXPECT_EQ(27, buffer.at(18));
            EXPECT_EQ(26, buffer.at(19));
            EXPECT_EQ(25, buffer.at(20));
            EXPECT_EQ(24, buffer.at(21));
            EXPECT_EQ(23, buffer.at(22));
            EXPECT_EQ(22, buffer.at(23));
            EXPECT_EQ(21, buffer.at(24));
            EXPECT_EQ(20, buffer.at(25));
            EXPECT_EQ(19, buffer.at(26));
            EXPECT_EQ(18, buffer.at(27));
            EXPECT_EQ(17, buffer.at(28));
            EXPECT_EQ(16, buffer.at(29));
            EXPECT_EQ(15, buffer.at(30));
            EXPECT_EQ(14, buffer.at(31));
            EXPECT_EQ(13, buffer.at(32));
            EXPECT_EQ(12, buffer.at(33));
            EXPECT_EQ(11, buffer.at(34));
            EXPECT_EQ(10, buffer.at(35));
            EXPECT_EQ(9, buffer.at(36));
            EXPECT_EQ(8, buffer.at(37));
            EXPECT_EQ(7, buffer.at(38));
            EXPECT_EQ(6, buffer.at(39));
            EXPECT_EQ(5, buffer.at(40));
            EXPECT_EQ(4, buffer.at(41));
            EXPECT_EQ(3, buffer.at(42));
            EXPECT_EQ(2, buffer.at(43));
            EXPECT_EQ(1, buffer.at(44));
            EXPECT_EQ(0, buffer.at(45));
        }
    }
}

TEST(MessagesTest, encode_versionResponsePayloadHighLevelHappyPath) {
    constexpr vcu::MessageType messages[]
        = { vcu::MessageType::DescribeConfigurationResponse, vcu::MessageType::DescribeConfigurationResponse };

    vcu::VersionResponsePayload expectedPayload;
    expectedPayload.sizeBytes = 0xfedcba9876543210LLU;
    std::iota(expectedPayload.checksumSha256.rbegin(), expectedPayload.checksumSha256.rend(), 0);

    // minimum size is 6 + 8 + 32 = 46
    constexpr uint16_t expectedSize = 46;
    std::vector<uint8_t> buffer(expectedSize, 0);
    for (auto m : messages) {
        for (auto t : targets) {
            vcu::Message msg;
            msg.type = m;
            msg.target = t;
            msg.versionResponsePayload = expectedPayload;

            std::vector<uint8_t> buffer(expectedSize, 0);

            EXPECT_EQ(vcu::CodecStatus::Success, vcu::encode(msg, buffer.begin(), buffer.end()));

            const uint16_t msgSize = (buffer.at(0) << 8) | buffer.at(1);
            EXPECT_EQ(expectedSize, msgSize);
            const uint16_t rawType = (buffer.at(2) << 8) | buffer.at(3);
            EXPECT_EQ(static_cast<std::underlying_type<vcu::MessageType>::type>(m), rawType);
            const uint16_t rawTarget = (buffer.at(4) << 8) | buffer.at(5);
            EXPECT_EQ(static_cast<std::underlying_type<vcu::EmbeddedUnitIdentifier>::type>(t), rawTarget);

            const uint64_t imageSize = (static_cast<uint64_t>(buffer.at(6)) << 56) | (static_cast<uint64_t>(buffer.at(7)) << 48)
                | (static_cast<uint64_t>(buffer.at(8)) << 40) | (static_cast<uint64_t>(buffer.at(9)) << 32)
                | (static_cast<uint64_t>(buffer.at(10)) << 24) | (static_cast<uint64_t>(buffer.at(11)) << 16)
                | (static_cast<uint64_t>(buffer.at(12)) << 8) | static_cast<uint64_t>(buffer.at(13));

            EXPECT_EQ(expectedPayload.sizeBytes, imageSize);

            EXPECT_EQ(31, buffer.at(14));
            EXPECT_EQ(30, buffer.at(15));
            EXPECT_EQ(29, buffer.at(16));
            EXPECT_EQ(28, buffer.at(17));
            EXPECT_EQ(27, buffer.at(18));
            EXPECT_EQ(26, buffer.at(19));
            EXPECT_EQ(25, buffer.at(20));
            EXPECT_EQ(24, buffer.at(21));
            EXPECT_EQ(23, buffer.at(22));
            EXPECT_EQ(22, buffer.at(23));
            EXPECT_EQ(21, buffer.at(24));
            EXPECT_EQ(20, buffer.at(25));
            EXPECT_EQ(19, buffer.at(26));
            EXPECT_EQ(18, buffer.at(27));
            EXPECT_EQ(17, buffer.at(28));
            EXPECT_EQ(16, buffer.at(29));
            EXPECT_EQ(15, buffer.at(30));
            EXPECT_EQ(14, buffer.at(31));
            EXPECT_EQ(13, buffer.at(32));
            EXPECT_EQ(12, buffer.at(33));
            EXPECT_EQ(11, buffer.at(34));
            EXPECT_EQ(10, buffer.at(35));
            EXPECT_EQ(9, buffer.at(36));
            EXPECT_EQ(8, buffer.at(37));
            EXPECT_EQ(7, buffer.at(38));
            EXPECT_EQ(6, buffer.at(39));
            EXPECT_EQ(5, buffer.at(40));
            EXPECT_EQ(4, buffer.at(41));
            EXPECT_EQ(3, buffer.at(42));
            EXPECT_EQ(2, buffer.at(43));
            EXPECT_EQ(1, buffer.at(44));
            EXPECT_EQ(0, buffer.at(45));
        }
    }
}

TEST(MessagesTest, encode_imageChunkPayloadLowLevelBadPayloadSize) {
    constexpr vcu::MessageType messages[] = { vcu::MessageType::FirmwareImageChunk, vcu::MessageType::ConfigurationImageChunk };

    vcu::ImageChunkPayload expectedPayload;
    expectedPayload.payloadSizeBytes = 1025;

    for (auto m : messages) {
        for (auto t : targets) {
            std::vector<uint8_t> buffer(2048);
            const auto payload = expectedPayload;
            EXPECT_EQ(vcu::CodecStatus::InvalidChunkSize, vcu::encode(m, t, payload, buffer.begin(), buffer.end()));
        }
    }
}

TEST(MessagesTest, encode_imageChunkPayloadLowLevelBufferEmpty) {
    constexpr vcu::MessageType messages[] = { vcu::MessageType::FirmwareImageChunk, vcu::MessageType::ConfigurationImageChunk };

    vcu::ImageChunkPayload expectedPayload;
    expectedPayload.payloadSizeBytes = 1;

    for (auto m : messages) {
        for (auto t : targets) {
            std::vector<uint8_t> buffer;
            const auto payload = expectedPayload;
            EXPECT_EQ(vcu::CodecStatus::BadBufferSize, vcu::encode(m, t, payload, buffer.begin(), buffer.end()));
        }
    }
}

TEST(MessagesTest, encode_imageChunkPayloadLowLevelBufferTooSmall) {
    constexpr vcu::MessageType messages[] = { vcu::MessageType::FirmwareImageChunk, vcu::MessageType::ConfigurationImageChunk };

    for (size_t payloadSizeBytes = 1; payloadSizeBytes <= 1024; ++payloadSizeBytes) {
        vcu::ImageChunkPayload expectedPayload;
        expectedPayload.payloadSizeBytes = payloadSizeBytes;
        for (auto m : messages) {
            for (auto t : targets) {
                std::vector<uint8_t> buffer(payloadSizeBytes + 8 - 1);
                const auto payload = expectedPayload;
                EXPECT_EQ(vcu::CodecStatus::BadBufferSize, vcu::encode(m, t, payload, buffer.begin(), buffer.end()));
            }
        }
    }
}

TEST(MessagesTest, encode_imageChunkPayloadLowLevelHappyPath) {
    constexpr vcu::MessageType messages[] = { vcu::MessageType::FirmwareImageChunk, vcu::MessageType::ConfigurationImageChunk };

    for (size_t payloadSizeBytes = 1; payloadSizeBytes <= 1024; ++payloadSizeBytes) {
        vcu::ImageChunkPayload expectedPayload;
        expectedPayload.payloadSizeBytes = payloadSizeBytes;
        std::fill(expectedPayload.imageBytes.begin(), expectedPayload.imageBytes.begin() + payloadSizeBytes, 0xff);
        std::fill(expectedPayload.imageBytes.begin() + payloadSizeBytes, expectedPayload.imageBytes.end(), 0);

        // 2 for packet length, 2 for message, 2 for target, 2 for payload size
        const uint16_t expectedSize = payloadSizeBytes + 8;

        for (auto m : messages) {
            for (auto t : targets) {
                std::vector<uint8_t> buffer(payloadSizeBytes + 8);
                const auto payload = expectedPayload;
                EXPECT_EQ(vcu::CodecStatus::Success, vcu::encode(m, t, payload, buffer.begin(), buffer.end()));
                const uint16_t msgSize = (buffer.at(0) << 8) | buffer.at(1);
                EXPECT_EQ(expectedSize, msgSize);
                const uint16_t rawType = (buffer.at(2) << 8) | buffer.at(3);
                EXPECT_EQ(static_cast<std::underlying_type<vcu::MessageType>::type>(m), rawType);
                const uint16_t rawTarget = (buffer.at(4) << 8) | buffer.at(5);
                EXPECT_EQ(static_cast<std::underlying_type<vcu::EmbeddedUnitIdentifier>::type>(t), rawTarget);
                const uint16_t payloadSize = (buffer.at(6) << 8) | buffer.at(7);
                EXPECT_EQ(payloadSizeBytes, payloadSize);
                for (size_t i = 0; i < payloadSizeBytes; ++i) {
                    EXPECT_EQ(0xff, buffer.at(i + 8));
                }
            }
        }
    }
}

TEST(MessagesTest, encode_imageChunkPayloadHighLevelHappyPath) {
    constexpr vcu::MessageType messages[] = { vcu::MessageType::FirmwareImageChunk, vcu::MessageType::ConfigurationImageChunk };

    for (size_t payloadSizeBytes = 1; payloadSizeBytes <= 1024; ++payloadSizeBytes) {
        vcu::ImageChunkPayload expectedPayload;
        expectedPayload.payloadSizeBytes = payloadSizeBytes;
        std::fill(expectedPayload.imageBytes.begin(), expectedPayload.imageBytes.begin() + payloadSizeBytes, 0xff);
        std::fill(expectedPayload.imageBytes.begin() + payloadSizeBytes, expectedPayload.imageBytes.end(), 0);

        // 2 for packet length, 2 for message, 2 for target, 2 for payload size
        const uint16_t expectedSize = payloadSizeBytes + 8;

        for (auto m : messages) {
            for (auto t : targets) {
                std::vector<uint8_t> buffer(expectedSize);
                vcu::Message msg;
                msg.type = m;
                msg.target = t;
                msg.imageChunkPayload = expectedPayload;

                EXPECT_EQ(vcu::CodecStatus::Success, vcu::encode(msg, buffer.begin(), buffer.end()));
                const uint16_t msgSize = (buffer.at(0) << 8) | buffer.at(1);
                EXPECT_EQ(expectedSize, msgSize);
                const uint16_t rawType = (buffer.at(2) << 8) | buffer.at(3);
                EXPECT_EQ(static_cast<std::underlying_type<vcu::MessageType>::type>(m), rawType);
                const uint16_t rawTarget = (buffer.at(4) << 8) | buffer.at(5);
                EXPECT_EQ(static_cast<std::underlying_type<vcu::EmbeddedUnitIdentifier>::type>(t), rawTarget);
                const uint16_t payloadSize = (buffer.at(6) << 8) | buffer.at(7);
                EXPECT_EQ(payloadSizeBytes, payloadSize);
                for (size_t i = 0; i < payloadSizeBytes; ++i) {
                    EXPECT_EQ(0xff, buffer.at(i + 8));
                }
            }
        }
    }
}

TEST(MessagesTest, encode_bootProgressPayloadLowLevelBufferEmpty) {
    vcu::BootProgressPayload expectedPayload;
    expectedPayload.totalTests = 20;
    expectedPayload.successfulTests = 10;
    expectedPayload.failedTests = 0;
    std::fill(expectedPayload.currentTestName.begin(), expectedPayload.currentTestName.end(), 0);
    expectedPayload.currentTestResult = vcu::BootUpSelfTestResult::Success;

    for (auto t : targets) {
        std::vector<uint8_t> buffer;
        const auto payload = expectedPayload;
        EXPECT_EQ(vcu::CodecStatus::BadBufferSize, vcu::encode(t, payload, buffer.begin(), buffer.end()));
    }
}

TEST(MessagesTest, encode_bootProgressPayloadLowLevelBufferTooSmall) {
    vcu::BootProgressPayload expectedPayload;
    expectedPayload.totalTests = 20;
    expectedPayload.successfulTests = 10;
    expectedPayload.failedTests = 0;
    std::fill(expectedPayload.currentTestName.begin(), expectedPayload.currentTestName.end(), 0);
    expectedPayload.currentTestResult = vcu::BootUpSelfTestResult::Success;

    for (auto t : targets) {
        std::vector<uint8_t> buffer(44);
        const auto payload = expectedPayload;
        EXPECT_EQ(vcu::CodecStatus::BadBufferSize, vcu::encode(t, payload, buffer.begin(), buffer.end()));
    }
}

TEST(MessagesTest, encode_bootProgressPayloadLowLevelHappyPath) {
    constexpr vcu::MessageType m = vcu::MessageType::BootUpSelfTestProgress;

    vcu::BootProgressPayload expectedPayload;
    expectedPayload.totalTests = 20;
    expectedPayload.successfulTests = 10;
    expectedPayload.failedTests = 0;
    std::fill(expectedPayload.currentTestName.begin(), expectedPayload.currentTestName.end(), 0);
    expectedPayload.currentTestResult = vcu::BootUpSelfTestResult::Success;

    constexpr size_t expectedSize = 45;
    for (auto t : targets) {
        std::vector<uint8_t> buffer(expectedSize);
        const auto payload = expectedPayload;
        EXPECT_EQ(vcu::CodecStatus::Success, vcu::encode(t, payload, buffer.begin(), buffer.end()));
        const uint16_t msgSize = (buffer.at(0) << 8) | buffer.at(1);
        EXPECT_EQ(expectedSize, msgSize);
        const uint16_t rawType = (buffer.at(2) << 8) | buffer.at(3);
        EXPECT_EQ(static_cast<std::underlying_type<vcu::MessageType>::type>(m), rawType);
        const uint16_t rawTarget = (buffer.at(4) << 8) | buffer.at(5);
        EXPECT_EQ(static_cast<std::underlying_type<vcu::EmbeddedUnitIdentifier>::type>(t), rawTarget);
        const uint16_t totalTests = (buffer.at(6) << 8) | buffer.at(7);
        EXPECT_EQ(expectedPayload.totalTests, totalTests);
        const uint16_t successfulTests = (buffer.at(8) << 8) | buffer.at(9);
        EXPECT_EQ(expectedPayload.successfulTests, successfulTests);
        const uint16_t failedTests = (buffer.at(10) << 8) | buffer.at(11);
        EXPECT_EQ(expectedPayload.failedTests, failedTests);
        for (size_t i = 0; i < expectedPayload.currentTestName.size(); ++i) {
            EXPECT_EQ(0, buffer.at(12 + i));
        }
        EXPECT_EQ(static_cast<std::underlying_type<vcu::BootUpSelfTestResult>::type>(expectedPayload.currentTestResult), buffer.back());
    }
}

TEST(MessagesTest, encode_bootProgressPayloadHighLevelHappyPath) {
    constexpr vcu::MessageType m = vcu::MessageType::BootUpSelfTestProgress;

    vcu::BootProgressPayload expectedPayload;
    expectedPayload.totalTests = 20;
    expectedPayload.successfulTests = 10;
    expectedPayload.failedTests = 0;
    std::fill(expectedPayload.currentTestName.begin(), expectedPayload.currentTestName.end(), 0);
    expectedPayload.currentTestResult = vcu::BootUpSelfTestResult::Success;

    constexpr size_t expectedSize = 45;
    for (auto t : targets) {
        std::vector<uint8_t> buffer(expectedSize);
        vcu::Message msg;
        msg.type = m;
        msg.target = t;
        msg.bootProgressPayload = expectedPayload;
        EXPECT_EQ(vcu::CodecStatus::Success, vcu::encode(msg, buffer.begin(), buffer.end()));
        const uint16_t msgSize = (buffer.at(0) << 8) | buffer.at(1);
        EXPECT_EQ(expectedSize, msgSize);
        const uint16_t rawType = (buffer.at(2) << 8) | buffer.at(3);
        EXPECT_EQ(static_cast<std::underlying_type<vcu::MessageType>::type>(m), rawType);
        const uint16_t rawTarget = (buffer.at(4) << 8) | buffer.at(5);
        EXPECT_EQ(static_cast<std::underlying_type<vcu::EmbeddedUnitIdentifier>::type>(t), rawTarget);
        const uint16_t totalTests = (buffer.at(6) << 8) | buffer.at(7);
        EXPECT_EQ(expectedPayload.totalTests, totalTests);
        const uint16_t successfulTests = (buffer.at(8) << 8) | buffer.at(9);
        EXPECT_EQ(expectedPayload.successfulTests, successfulTests);
        const uint16_t failedTests = (buffer.at(10) << 8) | buffer.at(11);
        EXPECT_EQ(expectedPayload.failedTests, failedTests);
        for (size_t i = 0; i < expectedPayload.currentTestName.size(); ++i) {
            EXPECT_EQ(0, buffer.at(12 + i));
        }
        EXPECT_EQ(static_cast<std::underlying_type<vcu::BootUpSelfTestResult>::type>(expectedPayload.currentTestResult), buffer.back());
    }
}

TEST(MessagesTest, encode_bootCompletedLowLevelEmptyBuffer) {
    vcu::BootCompletePayload expectedPayload;
    expectedPayload.testsFailed = 0;
    expectedPayload.testsPassed = 256;

    for (auto t : targets) {
        std::vector<uint8_t> buffer;
        const auto payload = expectedPayload;
        EXPECT_EQ(vcu::CodecStatus::BadBufferSize, vcu::encode(t, payload, buffer.begin(), buffer.end()));
    }
}

TEST(MessagesTest, encode_bootCompletedLowLevelSmallBuffer) {
    vcu::BootCompletePayload expectedPayload;
    expectedPayload.testsFailed = 0;
    expectedPayload.testsPassed = 256;

    for (auto t : targets) {
        std::vector<uint8_t> buffer(9, 0);
        const auto payload = expectedPayload;
        EXPECT_EQ(vcu::CodecStatus::BadBufferSize, vcu::encode(t, payload, buffer.begin(), buffer.end()));
    }
}

TEST(MessagesTest, encode_bootCompletedLowLevelHappyPath) {
    constexpr vcu::MessageType m = vcu::MessageType::BootUpSelfTestComplete;

    vcu::BootCompletePayload expectedPayload;
    expectedPayload.testsFailed = 0;
    expectedPayload.testsPassed = 256;

    constexpr size_t expectedSize = 10;
    for (auto t : targets) {
        std::vector<uint8_t> buffer(expectedSize, 0);
        const auto payload = expectedPayload;
        EXPECT_EQ(vcu::CodecStatus::Success, vcu::encode(t, payload, buffer.begin(), buffer.end()));
        const uint16_t msgSize = (buffer.at(0) << 8) | buffer.at(1);
        EXPECT_EQ(expectedSize, msgSize);
        const uint16_t rawType = (buffer.at(2) << 8) | buffer.at(3);
        EXPECT_EQ(static_cast<std::underlying_type<vcu::MessageType>::type>(m), rawType);
        const uint16_t rawTarget = (buffer.at(4) << 8) | buffer.at(5);
        EXPECT_EQ(static_cast<std::underlying_type<vcu::EmbeddedUnitIdentifier>::type>(t), rawTarget);
        const uint16_t testsPassed = (buffer.at(6) << 8) | buffer.at(7);
        EXPECT_EQ(expectedPayload.testsPassed, testsPassed);
        const uint16_t testsFailed = (buffer.at(8) << 8) | buffer.at(9);
        EXPECT_EQ(expectedPayload.testsFailed, testsFailed);
    }
}

TEST(MessagesTest, encode_bootCompletedHighLevelHappyPath) {
    constexpr vcu::MessageType m = vcu::MessageType::BootUpSelfTestComplete;

    vcu::BootCompletePayload expectedPayload;
    expectedPayload.testsFailed = 0;
    expectedPayload.testsPassed = 256;

    constexpr size_t expectedSize = 10;
    for (auto t : targets) {
        std::vector<uint8_t> buffer(expectedSize, 0);
        vcu::Message msg;
        msg.type = m;
        msg.target = t;
        msg.bootCompletePayload = expectedPayload;
        EXPECT_EQ(vcu::CodecStatus::Success, vcu::encode(msg, buffer.begin(), buffer.end()));
        const uint16_t msgSize = (buffer.at(0) << 8) | buffer.at(1);
        EXPECT_EQ(expectedSize, msgSize);
        const uint16_t rawType = (buffer.at(2) << 8) | buffer.at(3);
        EXPECT_EQ(static_cast<std::underlying_type<vcu::MessageType>::type>(m), rawType);
        const uint16_t rawTarget = (buffer.at(4) << 8) | buffer.at(5);
        EXPECT_EQ(static_cast<std::underlying_type<vcu::EmbeddedUnitIdentifier>::type>(t), rawTarget);
        const uint16_t testsPassed = (buffer.at(6) << 8) | buffer.at(7);
        EXPECT_EQ(expectedPayload.testsPassed, testsPassed);
        const uint16_t testsFailed = (buffer.at(8) << 8) | buffer.at(9);
        EXPECT_EQ(expectedPayload.testsFailed, testsFailed);
    }
}

TEST(MessagesTest, encode_selfTestPayloadLowLevelEmptyBuffer) {
    vcu::SelfTestPayload expectedPayload;
    expectedPayload.sendTimestampNanoseconds = 0x0123456789abcdefULL;
    expectedPayload.sequenceNumber = 0xfedcba9876543210ULL;

    for (auto t : targets) {
        std::vector<uint8_t> buffer;
        const auto payload = expectedPayload;
        EXPECT_EQ(vcu::CodecStatus::BadBufferSize, vcu::encode(t, payload, buffer.begin(), buffer.end()));
    }
}

TEST(MessagesTest, encode_selfTestPayloadLowLevelBufferTooSmall) {
    vcu::SelfTestPayload expectedPayload;
    expectedPayload.sendTimestampNanoseconds = 0x0123456789abcdefULL;
    expectedPayload.sequenceNumber = 0xfedcba9876543210ULL;

    for (auto t : targets) {
        std::vector<uint8_t> buffer(21);
        const auto payload = expectedPayload;
        EXPECT_EQ(vcu::CodecStatus::BadBufferSize, vcu::encode(t, payload, buffer.begin(), buffer.end()));
    }
}

TEST(MessagesTest, encode_selfTestPayloadLowLevelHappyPath) {
    constexpr vcu::MessageType m = vcu::MessageType::SelfTest;

    vcu::SelfTestPayload expectedPayload;
    expectedPayload.sendTimestampNanoseconds = 0x0123456789abcdefULL;
    expectedPayload.sequenceNumber = 0xfedcba9876543210ULL;

    constexpr size_t expectedSize = 22;
    for (auto t : targets) {
        std::vector<uint8_t> buffer(expectedSize);
        const auto payload = expectedPayload;
        EXPECT_EQ(vcu::CodecStatus::Success, vcu::encode(t, payload, buffer.begin(), buffer.end()));
        const uint16_t msgSize = (buffer.at(0) << 8) | buffer.at(1);
        EXPECT_EQ(expectedSize, msgSize);
        const uint16_t rawType = (buffer.at(2) << 8) | buffer.at(3);
        EXPECT_EQ(static_cast<std::underlying_type<vcu::MessageType>::type>(m), rawType);
        const uint16_t rawTarget = (buffer.at(4) << 8) | buffer.at(5);
        EXPECT_EQ(static_cast<std::underlying_type<vcu::EmbeddedUnitIdentifier>::type>(t), rawTarget);
        const uint64_t sequence = (static_cast<uint64_t>(buffer.at(6)) << 56) | (static_cast<uint64_t>(buffer.at(7)) << 48)
            | (static_cast<uint64_t>(buffer.at(8)) << 40) | (static_cast<uint64_t>(buffer.at(9)) << 32)
            | (static_cast<uint64_t>(buffer.at(10)) << 24) | (static_cast<uint64_t>(buffer.at(11)) << 16)
            | (static_cast<uint64_t>(buffer.at(12)) << 8) | buffer.at(13);
        EXPECT_EQ(expectedPayload.sequenceNumber, sequence);
        const uint64_t timestamp = (static_cast<uint64_t>(buffer.at(14)) << 56) | (static_cast<uint64_t>(buffer.at(15)) << 48)
            | (static_cast<uint64_t>(buffer.at(16)) << 40) | (static_cast<uint64_t>(buffer.at(17)) << 32)
            | (static_cast<uint64_t>(buffer.at(18)) << 24) | (static_cast<uint64_t>(buffer.at(19)) << 16)
            | (static_cast<uint64_t>(buffer.at(20)) << 8) | buffer.at(21);
        EXPECT_EQ(expectedPayload.sendTimestampNanoseconds, timestamp);
    }
}

TEST(MessagesTest, encode_selfTestPayloadHighLevelHappyPath) {
    constexpr vcu::MessageType m = vcu::MessageType::SelfTest;

    vcu::SelfTestPayload expectedPayload;
    expectedPayload.sendTimestampNanoseconds = 0x0123456789abcdefULL;
    expectedPayload.sequenceNumber = 0xfedcba9876543210ULL;

    constexpr size_t expectedSize = 22;
    for (auto t : targets) {
        std::vector<uint8_t> buffer(expectedSize);
        vcu::Message msg;
        msg.type = m;
        msg.target = t;
        msg.selfTestPayload = expectedPayload;

        EXPECT_EQ(vcu::CodecStatus::Success, vcu::encode(msg, buffer.begin(), buffer.end()));
        const uint16_t msgSize = (buffer.at(0) << 8) | buffer.at(1);
        EXPECT_EQ(expectedSize, msgSize);
        const uint16_t rawType = (buffer.at(2) << 8) | buffer.at(3);
        EXPECT_EQ(static_cast<std::underlying_type<vcu::MessageType>::type>(m), rawType);
        const uint16_t rawTarget = (buffer.at(4) << 8) | buffer.at(5);
        EXPECT_EQ(static_cast<std::underlying_type<vcu::EmbeddedUnitIdentifier>::type>(t), rawTarget);
        const uint64_t sequence = (static_cast<uint64_t>(buffer.at(6)) << 56) | (static_cast<uint64_t>(buffer.at(7)) << 48)
            | (static_cast<uint64_t>(buffer.at(8)) << 40) | (static_cast<uint64_t>(buffer.at(9)) << 32)
            | (static_cast<uint64_t>(buffer.at(10)) << 24) | (static_cast<uint64_t>(buffer.at(11)) << 16)
            | (static_cast<uint64_t>(buffer.at(12)) << 8) | buffer.at(13);
        EXPECT_EQ(expectedPayload.sequenceNumber, sequence);
        const uint64_t timestamp = (static_cast<uint64_t>(buffer.at(14)) << 56) | (static_cast<uint64_t>(buffer.at(15)) << 48)
            | (static_cast<uint64_t>(buffer.at(16)) << 40) | (static_cast<uint64_t>(buffer.at(17)) << 32)
            | (static_cast<uint64_t>(buffer.at(18)) << 24) | (static_cast<uint64_t>(buffer.at(19)) << 16)
            | (static_cast<uint64_t>(buffer.at(20)) << 8) | buffer.at(21);
        EXPECT_EQ(expectedPayload.sendTimestampNanoseconds, timestamp);
    }
}

TEST(MessagesTest, decode_emptyBuffer) {
    std::vector<uint8_t> buffer;
    vcu::Message msg;
    EXPECT_EQ(vcu::CodecStatus::BadBufferSize, vcu::decode(msg, buffer.begin(), buffer.end()));
}

TEST(MessagesTest, decode_invalidTarget) {
    std::array<uint8_t, 6> buffer{ { 0x00, 0x06, 0x00, 0x01, 0x00, 0x00 } }; // (6, DescribeFirmware, Unknown)
    vcu::Message msg;

    EXPECT_EQ(vcu::CodecStatus::BadTarget, vcu::decode(msg, buffer.begin(), buffer.end()));

    buffer[5] = 0xff; // (6, DescribeFirmware, Junk)

    EXPECT_EQ(vcu::CodecStatus::BadTarget, vcu::decode(msg, buffer.begin(), buffer.end()));
}

TEST(MessagesTest, decode_invalidMessage) {
    std::array<uint8_t, 6> buffer{ { 0x00, 0x06, 0x00, 0x00, 0x00, 0x01 } }; // (6, Unknown, LeftRear)
    vcu::Message msg;

    EXPECT_EQ(vcu::CodecStatus::BadMessageType, vcu::decode(msg, buffer.begin(), buffer.end()));

    buffer[3] = 0xff; // (6, Junk, LeftRear)

    EXPECT_EQ(vcu::CodecStatus::BadMessageType, vcu::decode(msg, buffer.begin(), buffer.end()));
}

TEST(MessagesTest, decodeHeaderOnlyMessagesIsIdempotent) {
    constexpr vcu::MessageType messages[] = { vcu::MessageType::DescribeConfiguration, vcu::MessageType::GetConfigurationImage,
        vcu::MessageType::DescribeFirmware, vcu::MessageType::GetFirmwareImage, vcu::MessageType::StartedBootingImage };

    for (auto m : messages) {
        for (auto t : targets) {
            vcu::Message original;
            original.type = m;
            original.target = t;
            // deliberately huge buffer
            std::vector<uint8_t> buffer(20, 0);
            EXPECT_EQ(vcu::CodecStatus::Success, vcu::encode(original, buffer.begin(), buffer.end()));

            vcu::Message decoded;
            EXPECT_EQ(vcu::CodecStatus::Success, vcu::decode(decoded, buffer.begin(), buffer.end()));
            EXPECT_EQ(original.type, decoded.type);
            EXPECT_EQ(original.target, decoded.target);
        }
    }
}

TEST(MessagesTest, decodeVersionResponsePayloadMessagesIsIdempotent) {
    constexpr vcu::MessageType messages[]
        = { vcu::MessageType::DescribeConfigurationResponse, vcu::MessageType::DescribeConfigurationResponse };

    vcu::VersionResponsePayload payload;
    payload.sizeBytes = 0xfedcba9876543210LLU;
    std::iota(payload.checksumSha256.rbegin(), payload.checksumSha256.rend(), 0);

    // minimum size is 6 + 8 + 32 = 46
    constexpr uint16_t expectedSize = 46;
    std::vector<uint8_t> buffer(expectedSize, 0);
    for (auto m : messages) {
        for (auto t : targets) {
            vcu::Message original;
            original.type = m;
            original.target = t;
            original.versionResponsePayload = payload;

            std::vector<uint8_t> buffer(expectedSize, 0);
            EXPECT_EQ(vcu::CodecStatus::Success, vcu::encode(original, buffer.begin(), buffer.end()));

            vcu::Message decoded;
            EXPECT_EQ(vcu::CodecStatus::Success, vcu::decode(decoded, buffer.begin(), buffer.end()));

            EXPECT_EQ(original.type, decoded.type);
            EXPECT_EQ(original.target, decoded.target);
            EXPECT_EQ(original.versionResponsePayload.sizeBytes, decoded.versionResponsePayload.sizeBytes);

            for (size_t i = 0; i < original.versionResponsePayload.checksumSha256.size(); ++i) {
                EXPECT_EQ(original.versionResponsePayload.checksumSha256[i], decoded.versionResponsePayload.checksumSha256[i]);
            }
        }
    }
}

TEST(MessagesTest, decodeImageChunkMessagesIsIdempotent) {
    constexpr vcu::MessageType messages[] = { vcu::MessageType::FirmwareImageChunk, vcu::MessageType::ConfigurationImageChunk };

    for (size_t payloadSizeBytes = 1; payloadSizeBytes <= 1024; ++payloadSizeBytes) {
        vcu::ImageChunkPayload payload;
        payload.payloadSizeBytes = payloadSizeBytes;
        std::fill(payload.imageBytes.begin(), payload.imageBytes.begin() + payloadSizeBytes, 0xff);
        std::fill(payload.imageBytes.begin() + payloadSizeBytes, payload.imageBytes.end(), 0);

        // 2 for packet length, 2 for message, 2 for target, 2 for payload size
        const uint16_t expectedSize = payloadSizeBytes + 8;

        for (auto m : messages) {
            for (auto t : targets) {
                std::vector<uint8_t> buffer(expectedSize);
                vcu::Message original;
                original.type = m;
                original.target = t;
                original.imageChunkPayload = payload;

                EXPECT_EQ(vcu::CodecStatus::Success, vcu::encode(original, buffer.begin(), buffer.end()));

                vcu::Message decoded;
                EXPECT_EQ(vcu::CodecStatus::Success, vcu::decode(decoded, buffer.begin(), buffer.end()));
                EXPECT_EQ(original.type, decoded.type);
                EXPECT_EQ(original.target, decoded.target);
                EXPECT_EQ(original.imageChunkPayload.payloadSizeBytes, decoded.imageChunkPayload.payloadSizeBytes);

                for (size_t i = 0; i < original.imageChunkPayload.payloadSizeBytes; ++i) {
                    EXPECT_EQ(original.imageChunkPayload.imageBytes[i], decoded.imageChunkPayload.imageBytes[i]);
                }
            }
        }
    }
}

TEST(MessagesTest, decodeBootProgressPayloadIsIdempotent) {
    constexpr vcu::MessageType m = vcu::MessageType::BootUpSelfTestProgress;

    vcu::BootProgressPayload payload;
    payload.totalTests = 20;
    payload.successfulTests = 10;
    payload.failedTests = 0;
    std::fill(payload.currentTestName.begin(), payload.currentTestName.end(), 0);
    payload.currentTestResult = vcu::BootUpSelfTestResult::Success;

    constexpr size_t expectedSize = 45;
    for (auto t : targets) {
        std::vector<uint8_t> buffer(expectedSize);
        vcu::Message original;
        original.type = m;
        original.target = t;
        original.bootProgressPayload = payload;

        EXPECT_EQ(vcu::CodecStatus::Success, vcu::encode(original, buffer.begin(), buffer.end()));
        vcu::Message decoded;
        EXPECT_EQ(vcu::CodecStatus::Success, vcu::decode(decoded, buffer.begin(), buffer.end()));
        EXPECT_EQ(original.type, decoded.type);
        EXPECT_EQ(original.target, decoded.target);

        EXPECT_EQ(original.bootProgressPayload.totalTests, decoded.bootProgressPayload.totalTests);
        EXPECT_EQ(original.bootProgressPayload.successfulTests, decoded.bootProgressPayload.successfulTests);
        EXPECT_EQ(original.bootProgressPayload.failedTests, decoded.bootProgressPayload.failedTests);

        for (size_t i = 0; i < original.bootProgressPayload.currentTestName.size(); ++i) {
            EXPECT_EQ(original.bootProgressPayload.currentTestName[i], decoded.bootProgressPayload.currentTestName[i]);
        }

        EXPECT_EQ(original.bootProgressPayload.currentTestResult, decoded.bootProgressPayload.currentTestResult);
    }
}

TEST(MessagesTest, decodeBootCompletePayloadIsIdempotent) {
    constexpr vcu::MessageType m = vcu::MessageType::BootUpSelfTestComplete;

    vcu::BootCompletePayload expectedPayload;
    expectedPayload.testsFailed = 0;
    expectedPayload.testsPassed = 256;

    constexpr size_t expectedSize = 10;
    for (auto t : targets) {
        std::vector<uint8_t> buffer(expectedSize, 0);
        vcu::Message original;
        original.type = m;
        original.target = t;
        original.bootCompletePayload = expectedPayload;
        EXPECT_EQ(vcu::CodecStatus::Success, vcu::encode(original, buffer.begin(), buffer.end()));

        vcu::Message decoded;
        EXPECT_EQ(vcu::CodecStatus::Success, vcu::decode(decoded, buffer.begin(), buffer.end()));
        EXPECT_EQ(original.type, decoded.type);
        EXPECT_EQ(original.target, decoded.target);
        EXPECT_EQ(original.bootCompletePayload.testsPassed, decoded.bootCompletePayload.testsPassed);
        EXPECT_EQ(original.bootCompletePayload.testsFailed, decoded.bootCompletePayload.testsFailed);
    }
}