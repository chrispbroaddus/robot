#pragma once

namespace unity_plugins {

enum class Status {
    Succeeded = 0,
    InProgress,
    Error_GlewInit,
    Error_UnsupportedAPI,
    Error_InvalidArguments,
    Error_TooManyRequests,
    Error_DownloadNoData,
    Error_DownloadWait,
    Error_UnsupportedPlatform,
    Error_UninitializedQueue
};
} // namespace unity_plugins
