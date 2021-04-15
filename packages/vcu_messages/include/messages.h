#pragma once

#include <algorithm>
#include <array>
#include <iterator>

namespace vcu {

/// ID of the various embedded units that will need to be flashed / monitored
enum struct EmbeddedUnitIdentifier : uint16_t {
    /// Default reserved to mean "unknown"
    Unknown,
    LeftRearMotorController,
    LeftFrontMotorController,
    RightRearMotorController,
    RightFrontMotorController,
    VehicleController
};

/// Message types exchanged between the various embedded units and the host
///
/// \par State Machine
/// \dot
/// digraph {
///   init [shape=circle,style=filled,fillcolor=black,label=""];
///   cfv[label="Check firmware version"];
///   df[label="Download firmware"];
///   dc[label="Download configuration"];
///   ccv[label="Check configuration version"];
///   sb[label="Start booting firmware"];
///   bust[label="Boot-up self test"];
///   bf[label="Boot finished"];
///   run[label="Running"];
///   init -> cfv;
///   cfv  -> ccv [label="Up-to-date"];
///   cfv  -> df [label="Need refresh"];
///   df   -> init [label="Reboot"];
///   ccv  -> sb [label="Up-to-date"];
///   ccv  -> dc [label="Need refresh"];
///   dc   -> init [label="Reboot"];
///   sb   -> bust [label="Start boot-up self test"];
///   bust -> bust [label="Boot-up self test progress"];
///   bust -> bf  [label="Boot-up self test complete"];
///   bf   -> run [label="First self-check message"];
///   run  -> run [label="Successful self-test"];
/// }
/// \enddot
///
/// \par Use Case: Firmware and configuration both already up-to-date
/// \msc
/// msc {
///   host, bootloader, xcufirmware;
///   bootloader abox bootloader [label="Open TCP connection to Host"];
///   |||;
///   --- [label = "Verify firmware version"];
///   |||;
///   bootloader->host [label = "describeFirmware(...)"];
///   host->bootloader [label = "describeFirmwareResponse(...)"];
///   bootloader=>bootloader [label = "checkFirmware(...): OK"];
///   |||;
///   --- [label = "Verify configuration version"];
///   |||;
///   bootloader->host [label = "describeConfiguration(...)"];
///   host->bootloader [label = "describeConfigurationResponse(...)"];
///   bootloader=>bootloader [label = "checkConfguration(...): OK"];
///   bootloader abox bootloader [label = "Close TCP connection"];
///   |||;
///   --- [label = "Boot firmware / boot-up self test"];
///   xcufirmware->host [label="startedBootingImage(...)"];
///   xcufirmware->host [label="bustProgress(...)"];
///   ...;
///   xcufirmware->host [label="bustComplete(...)"];
///   |||;
///   --- [label = "Periodic self-checks"];
///   xcufirmware->host [label="selfTest(...)"];
///   ...;
/// }
/// \endmsc
///
/// \par Use Case: Firmware needs to be updated.
/// \msc
/// msc {
///   host, bootloader;
///   |||;
///   bootloader abox bootloader [label="Connect to Host"];
///   |||;
///   --- [label = "Verify firmware version"];
///   |||;
///   bootloader->host [label = "describeFirmware(...)"];
///   host->bootloader [label = "describeFirmwareResponse(...)"];
///   bootloader=>bootloader [label = "checkFirmware(...): Needs update"];
///   |||;
///   --- [label = "Request and download new firmware"];
///   bootloader->host [label = "getFirmwareImage(...)"];
///   host->bootloader [label = "firmwareImageChunk(...)"];
///   ...;
///   host->bootloader [label = "firmwareImageChunk(...)"];
///   bootloader=>bootloader [label = "unpack firmware image"];
///   bootloader=>bootloader [label = "reboot" ];
/// }
/// \endmsc
///
/// \par Use Case: Configuration needs to be updated.
/// \msc
/// msc {
///   host, bootloader;
///   |||;
///   bootloader abox bootloader [label="Connect to Host"];
///   |||;
///   --- [label = "Verify firmware version"];
///   |||;
///   bootloader->host [label = "describeFirmware(...)"];
///   host->bootloader [label = "describeFirmwareResponse(...)"];
///   bootloader=>bootloader [label = "checkFirmware(...): OK"];
///   |||;
///   --- [label = "Verify configuration version"];
///   |||;
///   bootloader->host [label = "describeConfiguration(...)"];
///   host->bootloader [label = "describeConfigurationResponse(...)"];
///   bootloader=>bootloader [label = "checkConfguration(...): Needs update"];
///   |||;
///   --- [label = "Request and download firmware."];
///   bootloader->host [label = "getConfigurationImage(...)"];
///   host->bootloader [label = "configurationImageChunk(...)"];
///   ...;
///   host->bootloader [label = "configurationImageChunk(...)"];
///   bootloader=>bootloader [label = "unpack configuration image"];
///   bootloader=>bootloader [label = "reboot" ];
/// }
/// \endmsc
enum struct MessageType : uint16_t {
    /// Default reserved to mean "unknown"
    Unknown,

    /// @defgroup firmware_load Firmware / Configuration Loading
    ///
    /// All messages in this group are sent via TCP, with the xCU acting as a client
    /// of the proxy.
    /// @{

    /// Sent from xCU to host when it wants to know what firmware is available.
    /// If the sizes and checksums match as advertised, the xCU will assume that
    /// it has a good firmware image.
    DescribeFirmware,

    /// Sent in response to a DescribeFirmware
    DescribeFirmwareResponse,

    /// Request download of the firmware image
    GetFirmwareImage,

    /// Partial firmware image response. The assumption for this message is that because
    /// the complete firmware / configuration exchange occurs over TCP, we can ignore
    /// checksums because lower protocol levels will have handled that and we can also
    /// ignore the sequencing because TCP semantics.
    FirmwareImageChunk,

    /// Sent from xCU to host when it wants to know what configuration is available.
    DescribeConfiguration,

    /// Sent in response to DescribeConfiguration
    DescribeConfigurationResponse,

    /// Request download of the configuration image.
    GetConfigurationImage,

    /// Partial configuration image response. The assumption for this message is that because
    /// the complete firmware / configuration exchange occurs over TCP, we can ignore
    /// checksums because lower protocol levels will have handled that and we can also
    /// ignore the sequencing because TCP semantics.
    ConfigurationImageChunk,
    /// @}

    /// @defgroup bust_progress Boot-up Self Test Progress
    /// These are informative messages sent by the xCU via UDP. These messages are primarily intended
    /// to help with diagnostics / debugging.
    /// @{

    /// Optional. Sent from xCU to host when it starts booting the image. This should be the last
    /// time we see a message from the bootloader running on the xCU.
    StartedBootingImage,

    /// Optional. Sent periodically from xCU to host as it performs its BUST.
    BootUpSelfTestProgress,

    /// Optional. Sent from the xCU when BUST is complete.
    BootUpSelfTestComplete,
    /// @}

    /// @defgroup self_check Self-check / Watchdog messages.
    /// Contract:
    /// -# These should be sent by the xCU at least once every 100ms.
    /// -# If a message is more than 5ms late, it will be dropped.
    /// -# If more than one drop occurs within one second sliding window,
    /// the proxy will assume that the xCU is in a bad state and start screaming.
    /// .
    ///
    /// These messages should only be sent if the xCU has performed a successful self-check.
    /// Specifically, these messages should not "just" be sent from a background thread; they should
    /// only be sent if there is some meaningful indication of progress / health / sanity.
    SelfTest
};

enum struct BootUpSelfTestResult : uint8_t { Unknown, Failure, Success };

/// Common payload for MessageType::DescribeFirmwareResponse and
/// MessageType::DescribeConfigurationResponse.
struct VersionResponsePayload {
    uint64_t sizeBytes;
    std::array<uint8_t, 32> checksumSha256;
};

/// Payload for MessageType::FirmwareImageChunk and MessageType::ConfigurationImageChunk
struct ImageChunkPayload {
    /// This will be at most 1024.
    uint16_t payloadSizeBytes;

    /// Actual bytes transferred in this chunk.
    std::array<uint8_t, 1024> imageBytes;
};

/// Payload for MessageType::BootUpSelfTestProgress
struct BootProgressPayload {
    /// Total number of tests run as part of the BUST process
    uint16_t totalTests;

    /// Number of previously run tests that were successful.
    uint16_t successfulTests;

    /// Number of previouly run tests that failed.
    uint16_t failedTests;

    /// Up to 32 ASCII characters to describe the current test. If fewer than 32 characters are
    /// required, the remaining bytes should be zero-filled. If more are required, consult Strunk
    /// and White.
    std::array<uint8_t, 32> currentTestName;

    BootUpSelfTestResult currentTestResult;
};

/// Payload for MessageTypes::BootUpSelfTestComplete
struct BootCompletePayload {
    uint16_t testsPassed;
    uint16_t testsFailed;
};

/// Payload for MessageTypes::SelfTest
struct SelfTestPayload {
    /// Required to be monotonically increasing. Need to figure out what, if anything, the proxy
    /// will need to do to deal with wrap-around. Best bet is to start at 0 and increment.
    uint64_t sequenceNumber;

    /// Required to be monotonically increasing. We do not require that 0 mean anything; we do,
    /// however, require that differences between these quantities are "sane".
    uint64_t sendTimestampNanoseconds;
};

/// Message exchanged between the xCU and the host.
struct Message {
    MessageType type;
    EmbeddedUnitIdentifier target;
    union {
        VersionResponsePayload versionResponsePayload;
        ImageChunkPayload imageChunkPayload;
        BootProgressPayload bootProgressPayload;
        BootCompletePayload bootCompletePayload;
        SelfTestPayload selfTestPayload;
    };
};

enum struct CodecStatus {
    Unknown,
    /// Operation succeeded.
    Success,

    /// Operation failed. There was not enough space in the output buffer or enough data in the input buffer.
    BadBufferSize,

    /// Operation failed: unexpected value for msg.type
    BadMessageType,

    /// Operation failed: unexpected value for msg.target
    BadTarget,

    /// Operation failed: invalid chunk size (should be in the range [1, 1024])
    InvalidChunkSize,

    /// Operation failed: sizes are inconsistent or otherwise unexpected
    UnexpectedSize,

    /// Operation failed: decoded an invalid test result
    BadTestResult,
};

template <typename OUTPUT_ITERATOR>
CodecStatus encode(MessageType type, EmbeddedUnitIdentifier target, OUTPUT_ITERATOR begin, OUTPUT_ITERATOR end) {
    // 2 for size, 2 for type, 2 for target
    constexpr uint16_t minimumSize = 6;
    if (std::distance(begin, end) < minimumSize) {
        return CodecStatus::BadBufferSize;
    }

    if (vcu::MessageType::DescribeConfiguration != type && MessageType::DescribeFirmware != type
        && MessageType::GetConfigurationImage != type && MessageType::GetFirmwareImage != type
        && MessageType::StartedBootingImage != type) {
        return CodecStatus::BadMessageType;
    }

    *begin++ = (minimumSize >> 8) & 0xff;
    *begin++ = (minimumSize)&0xff;
    *begin++ = (static_cast<std::underlying_type<MessageType>::type>(type) >> 8) & 0xff;
    *begin++ = (static_cast<std::underlying_type<MessageType>::type>(type)) & 0xff;
    *begin++ = (static_cast<std::underlying_type<EmbeddedUnitIdentifier>::type>(target) >> 8) & 0xff;
    *begin++ = (static_cast<std::underlying_type<EmbeddedUnitIdentifier>::type>(target)) & 0xff;
    return CodecStatus::Success;
}

template <typename OUTPUT_ITERATOR>
CodecStatus encode(MessageType type, EmbeddedUnitIdentifier target, const VersionResponsePayload& versionResponse, OUTPUT_ITERATOR begin,
    OUTPUT_ITERATOR end) {
    // 2 for size, 2 for type, 2 for target, 8 for size, 32 for checksum
    constexpr uint16_t minimumSize = 46;

    if (std::distance(begin, end) < minimumSize) {
        return CodecStatus::BadBufferSize;
    }

    if (MessageType::DescribeConfigurationResponse != type && MessageType::DescribeFirmwareResponse != type) {
        return CodecStatus::BadMessageType;
    }

    *begin++ = (minimumSize >> 8) & 0xff;
    *begin++ = (minimumSize)&0xff;
    *begin++ = (static_cast<std::underlying_type<MessageType>::type>(type) >> 8) & 0xff;
    *begin++ = (static_cast<std::underlying_type<MessageType>::type>(type)) & 0xff;
    *begin++ = (static_cast<std::underlying_type<EmbeddedUnitIdentifier>::type>(target) >> 8) & 0xff;
    *begin++ = (static_cast<std::underlying_type<EmbeddedUnitIdentifier>::type>(target)) & 0xff;
    *begin++ = (versionResponse.sizeBytes >> 56) & 0xff;
    *begin++ = (versionResponse.sizeBytes >> 48) & 0xff;
    *begin++ = (versionResponse.sizeBytes >> 40) & 0xff;
    *begin++ = (versionResponse.sizeBytes >> 32) & 0xff;
    *begin++ = (versionResponse.sizeBytes >> 24) & 0xff;
    *begin++ = (versionResponse.sizeBytes >> 16) & 0xff;
    *begin++ = (versionResponse.sizeBytes >> 8) & 0xff;
    *begin++ = (versionResponse.sizeBytes) & 0xff;
    std::copy(versionResponse.checksumSha256.begin(), versionResponse.checksumSha256.end(), begin);
    return CodecStatus::Success;
}

template <typename OUTPUT_ITERATOR>
CodecStatus encode(
    MessageType type, EmbeddedUnitIdentifier target, const ImageChunkPayload& chunk, OUTPUT_ITERATOR begin, OUTPUT_ITERATOR end) {
    constexpr size_t maximumChunkSize = 1024;

    if (chunk.payloadSizeBytes > maximumChunkSize) {
        return CodecStatus::InvalidChunkSize;
    }

    if (MessageType::ConfigurationImageChunk != type && MessageType::FirmwareImageChunk != type) {
        return CodecStatus::BadMessageType;
    }

    // 2 for size, 2 for type, 2 for target, 2 chunk size,
    const uint16_t messageSize = 8 + chunk.payloadSizeBytes;

    if (std::distance(begin, end) < messageSize) {
        return CodecStatus::BadBufferSize;
    }

    *begin++ = (messageSize >> 8) & 0xff;
    *begin++ = (messageSize)&0xff;
    *begin++ = (static_cast<std::underlying_type<MessageType>::type>(type) >> 8) & 0xff;
    *begin++ = (static_cast<std::underlying_type<MessageType>::type>(type)) & 0xff;
    *begin++ = (static_cast<std::underlying_type<EmbeddedUnitIdentifier>::type>(target) >> 8) & 0xff;
    *begin++ = (static_cast<std::underlying_type<EmbeddedUnitIdentifier>::type>(target)) & 0xff;
    *begin++ = (chunk.payloadSizeBytes >> 8) & 0xff;
    *begin++ = (chunk.payloadSizeBytes) & 0xff;
    std::copy(chunk.imageBytes.begin(), chunk.imageBytes.begin() + chunk.payloadSizeBytes, begin);
    return CodecStatus::Success;
}

template <typename OUTPUT_ITERATOR>
CodecStatus encode(EmbeddedUnitIdentifier target, const BootProgressPayload& bootProgress, OUTPUT_ITERATOR begin, OUTPUT_ITERATOR end) {
    // 2 for size, 2 for type, 2 for target, 2 for total tests, 2 for succesful tests, 2 for failed tests, 32 for test name, 1 for result
    constexpr uint16_t minimumSize = 45;

    if (std::distance(begin, end) < minimumSize) {
        return CodecStatus::BadBufferSize;
    }

    *begin++ = (minimumSize >> 8) & 0xff;
    *begin++ = (minimumSize)&0xff;
    *begin++ = (static_cast<std::underlying_type<MessageType>::type>(MessageType::BootUpSelfTestProgress) >> 8) & 0xff;
    *begin++ = (static_cast<std::underlying_type<MessageType>::type>(MessageType::BootUpSelfTestProgress)) & 0xff;
    *begin++ = (static_cast<std::underlying_type<EmbeddedUnitIdentifier>::type>(target) >> 8) & 0xff;
    *begin++ = (static_cast<std::underlying_type<EmbeddedUnitIdentifier>::type>(target)) & 0xff;
    *begin++ = (bootProgress.totalTests >> 8) & 0xff;
    *begin++ = (bootProgress.totalTests) & 0xff;
    *begin++ = (bootProgress.successfulTests >> 8) & 0xff;
    *begin++ = (bootProgress.successfulTests) & 0xff;
    *begin++ = (bootProgress.failedTests >> 8) & 0xff;
    *begin++ = (bootProgress.failedTests) & 0xff;

    for (size_t i = 0; i < bootProgress.currentTestName.size(); ++i) {
        *begin++ = bootProgress.currentTestName[i];
    }

    *begin++ = static_cast<std::underlying_type<BootUpSelfTestResult>::type>(bootProgress.currentTestResult);
    return CodecStatus::Success;
}

template <typename OUTPUT_ITERATOR>
CodecStatus encode(EmbeddedUnitIdentifier target, const BootCompletePayload& bootComplete, OUTPUT_ITERATOR begin, OUTPUT_ITERATOR end) {
    // 2 for size, 2 for type, 2 for target, 2 for passes, 2 for fails
    constexpr uint16_t minimumSize = 10;

    if (std::distance(begin, end) < minimumSize) {
        return CodecStatus::BadBufferSize;
    }

    *begin++ = (minimumSize >> 8) & 0xff;
    *begin++ = (minimumSize)&0xff;
    *begin++ = (static_cast<std::underlying_type<MessageType>::type>(MessageType::BootUpSelfTestComplete) >> 8) & 0xff;
    *begin++ = (static_cast<std::underlying_type<MessageType>::type>(MessageType::BootUpSelfTestComplete)) & 0xff;
    *begin++ = (static_cast<std::underlying_type<EmbeddedUnitIdentifier>::type>(target) >> 8) & 0xff;
    *begin++ = (static_cast<std::underlying_type<EmbeddedUnitIdentifier>::type>(target)) & 0xff;
    *begin++ = (bootComplete.testsPassed >> 8) & 0xff;
    *begin++ = (bootComplete.testsPassed) & 0xff;
    *begin++ = (bootComplete.testsFailed >> 8) & 0xff;
    *begin++ = (bootComplete.testsFailed) & 0xff;
    return CodecStatus::Success;
}

template <typename OUTPUT_ITERATOR>
CodecStatus encode(EmbeddedUnitIdentifier target, const SelfTestPayload& selfTest, OUTPUT_ITERATOR begin, OUTPUT_ITERATOR end) {

    // 2 for size, 2 for type, 2 for target, 8 for sequence number, 8 for timestamp
    constexpr uint16_t minimumSize = 22;

    if (std::distance(begin, end) < minimumSize) {
        return CodecStatus::BadBufferSize;
    }

    *begin++ = (minimumSize >> 8) & 0xff;
    *begin++ = (minimumSize)&0xff;
    *begin++ = (static_cast<std::underlying_type<MessageType>::type>(MessageType::SelfTest) >> 8) & 0xff;
    *begin++ = (static_cast<std::underlying_type<MessageType>::type>(MessageType::SelfTest)) & 0xff;
    *begin++ = (static_cast<std::underlying_type<EmbeddedUnitIdentifier>::type>(target) >> 8) & 0xff;
    *begin++ = (static_cast<std::underlying_type<EmbeddedUnitIdentifier>::type>(target)) & 0xff;
    *begin++ = (selfTest.sequenceNumber >> 56) & 0xff;
    *begin++ = (selfTest.sequenceNumber >> 48) & 0xff;
    *begin++ = (selfTest.sequenceNumber >> 40) & 0xff;
    *begin++ = (selfTest.sequenceNumber >> 32) & 0xff;
    *begin++ = (selfTest.sequenceNumber >> 24) & 0xff;
    *begin++ = (selfTest.sequenceNumber >> 16) & 0xff;
    *begin++ = (selfTest.sequenceNumber >> 8) & 0xff;
    *begin++ = (selfTest.sequenceNumber) & 0xff;
    *begin++ = (selfTest.sendTimestampNanoseconds >> 56) & 0xff;
    *begin++ = (selfTest.sendTimestampNanoseconds >> 48) & 0xff;
    *begin++ = (selfTest.sendTimestampNanoseconds >> 40) & 0xff;
    *begin++ = (selfTest.sendTimestampNanoseconds >> 32) & 0xff;
    *begin++ = (selfTest.sendTimestampNanoseconds >> 24) & 0xff;
    *begin++ = (selfTest.sendTimestampNanoseconds >> 16) & 0xff;
    *begin++ = (selfTest.sendTimestampNanoseconds >> 8) & 0xff;
    *begin++ = (selfTest.sendTimestampNanoseconds) & 0xff;

    return CodecStatus::Success;
}

/// @tparam OUTPUT_ITERATOR Output iterator type. Note, this can be uint8_t*
///
/// @param msg
/// Message to encode
///
/// @param begin
/// Iterator to start of output buffer.
///
/// @param end
/// Iterator to end of output buffer. Note that std::difference(end, begin) is assumed to give
/// us the maximum usable portion of the buffer.
///
template <typename OUTPUT_ITERATOR> CodecStatus encode(const Message& msg, OUTPUT_ITERATOR begin, OUTPUT_ITERATOR end) {
    switch (msg.type) {
    // Messages with no payload (fallthrough)
    case MessageType::DescribeFirmware:
    case MessageType::GetFirmwareImage:
    case MessageType::DescribeConfiguration:
    case MessageType::GetConfigurationImage:
    case MessageType::StartedBootingImage:
        return encode(msg.type, msg.target, begin, end);

    // Messages with a version response payload (fallthrough)
    case MessageType::DescribeFirmwareResponse:
    case MessageType::DescribeConfigurationResponse:
        return encode(msg.type, msg.target, msg.versionResponsePayload, begin, end);

    // Messages with an image chunk payload (fallthrough)
    case MessageType::FirmwareImageChunk:
    case MessageType::ConfigurationImageChunk:
        return encode(msg.type, msg.target, msg.imageChunkPayload, begin, end);

    case MessageType::BootUpSelfTestProgress:
        return encode(msg.target, msg.bootProgressPayload, begin, end);

    case MessageType::BootUpSelfTestComplete:
        return encode(msg.target, msg.bootCompletePayload, begin, end);

    case MessageType::SelfTest:
        return encode(msg.target, msg.selfTestPayload, begin, end);

    default:
        // Not a message type we understand (yet?)
        return CodecStatus::BadMessageType;
    }
}

constexpr bool isValidEmbeddedUnitIdentifier(EmbeddedUnitIdentifier id) {
    return (id == EmbeddedUnitIdentifier::LeftFrontMotorController || id == EmbeddedUnitIdentifier::RightFrontMotorController
        || id == EmbeddedUnitIdentifier::LeftRearMotorController || id == EmbeddedUnitIdentifier::RightRearMotorController
        || id == EmbeddedUnitIdentifier::VehicleController);
}

template <typename INPUT_ITERATOR> INPUT_ITERATOR decode(uint16_t& out, INPUT_ITERATOR begin) {
    out = (*begin) << 8;
    ++begin;
    out |= (*begin);
    return ++begin;
}

template <typename INPUT_ITERATOR> INPUT_ITERATOR decode(uint64_t& out, INPUT_ITERATOR begin) {
    // Use horner's rule to reconstruct out
    out = (*begin);
    ++begin;
    out <<= 8;
    out |= *begin;
    ++begin;
    out <<= 8;
    out |= *begin;
    ++begin;
    out <<= 8;
    out |= *begin;
    ++begin;
    out <<= 8;
    out |= *begin;
    ++begin;
    out <<= 8;
    out |= *begin;
    ++begin;
    out <<= 8;
    out |= *begin;
    ++begin;
    out <<= 8;
    out |= *begin;
    return ++begin;
}

template <typename INPUT_ITERATOR>
CodecStatus decode(VersionResponsePayload& payload, uint16_t totalSize, INPUT_ITERATOR begin, INPUT_ITERATOR /*end*/) {
    if (totalSize != 6 + 8 + 32) {
        return CodecStatus::BadBufferSize;
    }

    begin = decode(payload.sizeBytes, begin);

    std::copy(begin, begin + payload.checksumSha256.size(), payload.checksumSha256.begin());
    return CodecStatus::Success;
}

template <typename INPUT_ITERATOR>
CodecStatus decode(ImageChunkPayload& payload, uint16_t totalSize, INPUT_ITERATOR begin, INPUT_ITERATOR /*end*/) {
    begin = decode(payload.payloadSizeBytes, begin);

    if (payload.payloadSizeBytes > 1024) {
        return CodecStatus::InvalidChunkSize;
    }

    if (payload.payloadSizeBytes + 8 != totalSize) {
        return CodecStatus::UnexpectedSize;
    }

    std::copy(begin, begin + payload.payloadSizeBytes, payload.imageBytes.begin());
    return CodecStatus::Success;
}

template <typename INPUT_ITERATOR>
CodecStatus decode(BootProgressPayload& payload, uint16_t totalSize, INPUT_ITERATOR begin, INPUT_ITERATOR /*end*/) {
    constexpr uint16_t expectedSize = 6 + 3 * 2 + 32 + 1; // 6 (header), 2 x (total, successful, failed), 32 for name, 1 for result

    if (totalSize != expectedSize) {
        return CodecStatus::UnexpectedSize;
    }

    begin = decode(payload.totalTests, begin);
    begin = decode(payload.successfulTests, begin);
    begin = decode(payload.failedTests, begin);

    for (size_t i = 0; i < payload.currentTestName.size(); ++i) {
        payload.currentTestName[i] = *(begin++);
    }

    payload.currentTestResult = static_cast<BootUpSelfTestResult>(*begin++);

    switch (payload.currentTestResult) {
    // Fall through is deliberate
    case BootUpSelfTestResult::Success:
    case BootUpSelfTestResult::Failure:
        return CodecStatus::Success;
    default:
        return CodecStatus::BadTestResult;
    }
}

template <typename INPUT_ITERATOR>
CodecStatus decode(BootCompletePayload& payload, uint16_t totalSize, INPUT_ITERATOR begin, INPUT_ITERATOR /*end*/) {
    constexpr uint16_t expectedSize = 6 + 2 * 2; // 6 (header), 2 for passes, 2 for fails

    if (totalSize != expectedSize) {
        return CodecStatus::UnexpectedSize;
    }

    begin = decode(payload.testsPassed, begin);
    begin = decode(payload.testsFailed, begin);

    return CodecStatus::Success;
}

template <typename INPUT_ITERATOR>
CodecStatus decode(SelfTestPayload& payload, uint16_t totalSize, INPUT_ITERATOR begin, INPUT_ITERATOR /*end*/) {
    constexpr uint16_t expectedSize = 6 + 16;

    if (expectedSize != totalSize) {
        return CodecStatus::UnexpectedSize;
    }

    begin = decode(payload.sequenceNumber, begin);
    begin = decode(payload.sendTimestampNanoseconds, begin);

    return CodecStatus::Success;
}

/// @tparam INPUT_ITERATOR
/// Input iterator type. Note, this can be const uint8_t*
///
/// @param [out] msg
/// Output message. Note, because we parse incrementally, regardless of whether or not we are able
/// parse a complete message, msg will be modified in-place.
///
/// @param begin
///
/// @param end
template <typename INPUT_ITERATOR> CodecStatus decode(Message& msg, INPUT_ITERATOR begin, INPUT_ITERATOR end) {
    const size_t numElements = std::distance(begin, end);

    // Do we have enough to decode the smallest possible messages? (2 for size, 2 for type, 2 for target)
    if (numElements < 6) {
        return CodecStatus::BadBufferSize;
    }

    uint16_t tmp;
    begin = decode(tmp, begin);
    const uint16_t totalSize = tmp;

    if (numElements < totalSize) {
        return CodecStatus::BadBufferSize;
    }

    begin = decode(tmp, begin);
    msg.type = static_cast<MessageType>(tmp);

    begin = decode(tmp, begin);
    msg.target = static_cast<EmbeddedUnitIdentifier>(tmp);

    if (isValidEmbeddedUnitIdentifier(msg.target)) {
        switch (msg.type) {
        // Messages with no payload (fallthrough)
        case MessageType::DescribeFirmware:
        case MessageType::GetFirmwareImage:
        case MessageType::DescribeConfiguration:
        case MessageType::GetConfigurationImage:
        case MessageType::StartedBootingImage:
            return CodecStatus::Success;

        // Messages with a version response payload (fallthrough)
        case MessageType::DescribeFirmwareResponse:
        case MessageType::DescribeConfigurationResponse:
            return decode(msg.versionResponsePayload, totalSize, begin, end);

        // Messages with an image chunk payload (fallthrough)
        case MessageType::FirmwareImageChunk:
        case MessageType::ConfigurationImageChunk:
            return decode(msg.imageChunkPayload, totalSize, begin, end);

        case MessageType::BootUpSelfTestProgress:
            return decode(msg.bootProgressPayload, totalSize, begin, end);

        case MessageType::BootUpSelfTestComplete:
            return decode(msg.bootCompletePayload, totalSize, begin, end);

        case MessageType::SelfTest:
            return decode(msg.selfTestPayload, totalSize, begin, end);

        default:
            // Not a message type we understand (yet?)
            return CodecStatus::BadMessageType;
        }
    } else {
        return CodecStatus::BadTarget;
    }
}
}