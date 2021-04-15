genrule(
    name = "download_saved_model_half_plus_two",
    srcs = [],
    outs = ["saved_model_half_plus_two"],
    cmd = "aws s3 cp s3://zippy.ml.models/saved_model_half_plus_two.tar.gz tmp.tar.gz &&" +
          "mkdir -p \"$@\" &&" +
          "tar -zxvf tmp.tar.gz -C \"$@\"",
    tags = ["manual"],
    visibility = ["//visibility:public"],
)

genrule(
    name = "download_pretrained_faster_rcnn_resnet101_model_data",
    srcs = [],
    outs = ["faster_rcnn_resnet101_coco_11_06_2017"],
    cmd = "aws s3 cp s3://zippy.ml.models/faster_rcnn_resnet101_coco_11_06_2017.tar.gz tmp.tar.gz &&" +
          "mkdir -p \"$@\" &&" +
          "tar -zxvf tmp.tar.gz -C \"$@\"",
    tags = ["manual"],
    visibility = ["//visibility:public"],
)

genrule(
    name = "download_pretrained_ssd_mobilenet_v1_model_data",
    srcs = [],
    outs = ["ssd_mobilenet_v1_coco_11_06_2017"],
    cmd = "aws s3 cp s3://zippy.ml.models/ssd_mobilenet_v1_coco_11_06_2017.tar.gz tmp.tar.gz &&" +
          "mkdir -p \"$@\" &&" +
          "tar -zxvf tmp.tar.gz -C \"$@\"",
    tags = ["manual"],
    visibility = ["//visibility:public"],
)

genrule(
    name = "download_pretrained_ssd_inception_v2_model_data",
    srcs = [],
    outs = ["ssd_inception_v2_coco_11_06_2017"],
    cmd = "aws s3 cp s3://zippy.ml.models/ssd_inception_v2_coco_11_06_2017.tar.gz tmp.tar.gz &&" +
          "mkdir -p \"$@\" &&" +
          "tar -zxvf tmp.tar.gz -C \"$@\"",
    tags = ["manual"],
    visibility = ["//visibility:public"],
)

genrule(
    name = "download_pretrained_rfcn_resnet101_model_data",
    srcs = [],
    outs = ["rfcn_resnet101_coco_11_06_2017"],
    cmd = "aws s3 cp s3://zippy.ml.models/rfcn_resnet101_coco_11_06_2017.tar.gz tmp.tar.gz &&" +
          "mkdir -p \"$@\" &&" +
          "tar -zxvf tmp.tar.gz -C \"$@\"",
    tags = ["manual"],
    visibility = ["//visibility:public"],
)

genrule(
    name = "download_pretrained_faster_rcnn_inception_resnet_v2_atrous_model_data",
    srcs = [],
    outs = ["faster_rcnn_inception_resnet_v2_coco_11_06_2017"],
    cmd = "aws s3 cp s3://zippy.ml.models/faster_rcnn_inception_resnet_v2_atrous_coco_11_06_2017.tar.gz tmp.tar.gz &&" +
          "mkdir -p \"$@\" &&" +
          "tar -zxvf tmp.tar.gz -C \"$@\"",
    tags = ["manual"],
    visibility = ["//visibility:public"],
)
