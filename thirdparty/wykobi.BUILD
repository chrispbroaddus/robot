licenses(["permissive"])
# A slim 2d/3d geometry library:
# http://www.wykobi.com/
# Permissive (MIT) license

cc_library(
    name = "wykobi",
    srcs = [
      "wykobi/wykobi_build.cpp",
      "wykobi/wykobi_algorithm.hpp",
      "wykobi/wykobi_graphics_net.hpp",
      "wykobi/wykobi_graphics_opengl.hpp",
      "wykobi/wykobi_graphics_vcl.hpp",
      "wykobi/wykobi_gui.hpp",
      "wykobi/wykobi.hpp",
      "wykobi/wykobi_instantiate.hpp",
      "wykobi/wykobi_math.hpp",
      "wykobi/wykobi_matrix.hpp",
      "wykobi/wykobi_nd.hpp",
      "wykobi/wykobi_utilities.hpp",
      ],
    visibility = ["//visibility:public"],
    hdrs = [
      "wykobi/wykobi_axis_projection_descriptor.inl",
      "wykobi/wykobi_clipping.inl",
      "wykobi/wykobi_duplicates.inl",
      "wykobi/wykobi_earclipping.inl",
      "wykobi/wykobi_hull.inl",
      "wykobi/wykobi.inl",
      "wykobi/wykobi_matrix.inl",
      "wykobi/wykobi_minimum_bounding_ball.inl",
      "wykobi/wykobi_naive_group_intersections.inl",
      "wykobi/wykobi_nd.inl",
      "wykobi/wykobi_normalization.inl",
      "wykobi/wykobi_ordered_polygon.inl",
    ],
    copts = ["-Wno-misleading-indentation"]
)
