load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_library(
    name = "utils",
    srcs = ["src/utils.cpp"],
    hdrs = ["include/utils.h"],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//applications/object_detection_evaluator/proto:metrics",
        "//external:glog",
        "//packages/machine_learning:detection_utils",
        "//packages/perception/proto:detection",
    ],
)

py_binary(
    name = "annotation_converter",
    srcs = [
        "convert_annotation_via_to_zippy.py",
    ],
    default_python_version = "PY3",
    main = "convert_annotation_via_to_zippy.py",
    srcs_version = "PY3",
    deps = [
        "//packages/perception/proto:dataset_py",
        "//packages/perception/proto:detection_py",
        "@com_github_google_protobuf//:protobuf_python",
    ],
)

cc_binary(
    name = "evaluator",
    srcs = ["main.cpp"],
    copts = COPTS,
    deps = [
        "//applications/object_detection_evaluator:utils",
        "//applications/object_detection_evaluator/proto:metrics",
        "//external:gflags",
        "//external:glog",
        "//external:opencv",
        "//packages/hal/proto:camera_sample",
        "//packages/machine_learning:object_detector",
        "//packages/perception/proto:dataset",
        "//packages/utilities/opencv:opencv_utilities",
        "@libtensorflow//:tensorflow",
    ],
)

genrule(
    name = "run_evaluator",
    outs = ["summary_ssd_mobilenet.json"],
    cmd = """
    $(location :evaluator) \
    -dataset $(location //packages/machine_learning/dataset:TinyEvalData_v0.0.1) \
    -annotation $(location //packages/machine_learning/dataset:TinyEvalData_v0.0.1)/zippy_annotations.json \
    -model $(location //packages/machine_learning/models:download_pretrained_ssd_mobilenet_v1_model_data)/saved \
    -output $@
    if [ -n "$$BRANCH" ] && [ -n "$$COMMIT" ]; then aws s3 cp $@ s3://zippy.ml.eval/$$BRANCH/$$COMMIT.json; fi
    """,
    tags = [
        "manual",
    ],
    tools = [
        ":evaluator",
        "//packages/machine_learning/dataset:TinyEvalData_v0.0.1",
        "//packages/machine_learning/models:download_pretrained_ssd_mobilenet_v1_model_data",
    ],
)

genrule(
    name = "run_evaluator_baseline_ssd_inception",
    outs = ["summary_ssd_inception.json"],
    cmd = """
    $(location :evaluator) \
    -dataset $(location //packages/machine_learning/dataset:TinyEvalData_v0.0.1) \
    -annotation $(location //packages/machine_learning/dataset:TinyEvalData_v0.0.1)/zippy_annotations.json \
    -model $(location //packages/machine_learning/models:download_pretrained_ssd_inception_v2_model_data)/saved \
    -output $@
    """,
    tags = [
        "manual",
    ],
    tools = [
        ":evaluator",
        "//packages/machine_learning/dataset:TinyEvalData_v0.0.1",
        "//packages/machine_learning/models:download_pretrained_ssd_inception_v2_model_data",
    ],
)

genrule(
    name = "run_evaluator_baseline_rfcn_resnet101",
    outs = ["summary_rfcn_resnet101.json"],
    cmd = """
    $(location :evaluator) \
    -dataset $(location //packages/machine_learning/dataset:TinyEvalData_v0.0.1) \
    -annotation $(location //packages/machine_learning/dataset:TinyEvalData_v0.0.1)/zippy_annotations.json \
    -model $(location //packages/machine_learning/models:download_pretrained_rfcn_resnet101_model_data)/saved \
    -output $@
    """,
    tags = [
        "manual",
    ],
    tools = [
        ":evaluator",
        "//packages/machine_learning/dataset:TinyEvalData_v0.0.1",
        "//packages/machine_learning/models:download_pretrained_rfcn_resnet101_model_data",
    ],
)

genrule(
    name = "run_evaluator_baseline_faster_rcnn_resnet101",
    outs = ["summary_faster_rcnn_resnet101.json"],
    cmd = """
    $(location :evaluator) \
    -dataset $(location //packages/machine_learning/dataset:TinyEvalData_v0.0.1) \
    -annotation $(location //packages/machine_learning/dataset:TinyEvalData_v0.0.1)/zippy_annotations.json \
    -model $(location //packages/machine_learning/models:download_pretrained_faster_rcnn_resnet101_model_data)/saved \
    -output $@
    """,
    tags = [
        "manual",
    ],
    tools = [
        ":evaluator",
        "//packages/machine_learning/dataset:TinyEvalData_v0.0.1",
        "//packages/machine_learning/models:download_pretrained_faster_rcnn_resnet101_model_data",
    ],
)

genrule(
    name = "run_evaluator_baseline_faster_rcnn_inception_resnet",
    outs = ["summary_faster_rcnn_inception_resnet.json"],
    cmd = """
    $(location :evaluator) \
    -dataset $(location //packages/machine_learning/dataset:TinyEvalData_v0.0.1) \
    -annotation $(location //packages/machine_learning/dataset:TinyEvalData_v0.0.1)/zippy_annotations.json \
    -model $(location //packages/machine_learning/models:download_pretrained_faster_rcnn_inception_resnet_v2_atrous_model_data)/saved \
    -output $@
    """,
    tags = [
        "manual",
    ],
    tools = [
        ":evaluator",
        "//packages/machine_learning/dataset:TinyEvalData_v0.0.1",
        "//packages/machine_learning/models:download_pretrained_faster_rcnn_inception_resnet_v2_atrous_model_data",
    ],
)
