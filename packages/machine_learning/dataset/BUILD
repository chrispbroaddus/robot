genrule(
    name = "TinyEvalData_v0.0.1",
    srcs = [],
    outs = ["TinyEvalData"],
    cmd = "aws s3 cp s3://zippy.data/TinyEvalData.v.0.0.1.tar.gz tmp.tar.gz &&" +
          "mkdir -p \"$@\" &&" +
          "tar -zxvf tmp.tar.gz -C \"$@\"",
    tags = ["manual"],
    visibility = ["//visibility:public"],
)
