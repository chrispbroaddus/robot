load("//tools:cpp_compile_flags.bzl", "COPTS")

COMMON_COPTS = COPTS

genrule(
    name = "regression_test_correct_messages_vcu",
    srcs = [],
    outs = ["vcu-2017-08-22T23_00_38_protodat"],
    cmd = "aws s3 cp s3://zippy-build-test/regression/packages/estimation/2017-08-22T23:00:37/vcu/vcu-2017-08-22T23:00:38.protodat tmp && cat tmp > \"$@\"",
    tags = ["manual"],
)

genrule(
    name = "regression_test_corrupt_messages_vcu",
    srcs = [],
    outs = ["vcu-2017-08-29T19_25_00.protodat"],
    cmd = "aws s3 cp s3://zippy-build-test/regression/packages/estimation/2017-08-29T19:25:00/vcu/vcu-2017-08-29T19:25:00.protodat  tmp && cat tmp > \"$@\"",
    tags = ["manual"],
)

cc_test(
    name = "estimator_test",
    srcs = ["estimator_test.cpp"],
    copts = COMMON_COPTS,
    data = [
        ":regression_test_correct_messages_vcu",
        ":regression_test_corrupt_messages_vcu",
    ],
    tags = ["manual"],
    deps = [
        "//packages/core/test:common",
        "//packages/data_logger/proto:config",
        "//packages/estimation",
        "//packages/filesystem",
        "//packages/hal",
        "//packages/serialization",
        "@gtest//:main",
    ],
)
