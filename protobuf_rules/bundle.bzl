load("//protobuf_rules/cpp:rules.bzl", "cc_proto_library")
load("//protobuf_rules/python:rules.bzl", "py_proto_library")

# This skylark
# macro(https://docs.bazel.build/versions/master/skylark/macros.html)
# will help translate .proto file into both C++ files and python files
# by calling cc_proto_library and py_proto_library bzl functions.The
# cc_proto_library will be named the same the given bundle name, while
# the corresponding py_library target will be named as name + "_py".
#
# At the moment the languages argument only supports a subset of
# ("c++", "python"), adding other languages like go, java should be as
# easy as loading the corresponding lang_proto_library bzl definition
# and expanding the for loop below.
#
# Notes about deps: all the transitive deps of a given
# proto_library_bundle should at least have the same set of languages
# defined otherwise there will be missing dependency.
def proto_library_bundle(
    name,
    deps=[],
    protos=[],
    languages=["c++", "python"],
    python_src_version="PY2AND3",
    **kwargs):
    for lang in languages:
        if lang == "c++":
            cc_proto_library(
                name=name,
                proto_deps=deps,
                protos=protos,
                **kwargs)
        elif lang == "python":
            py_proto_library(
                name=name + "_py",
                proto_deps=[dep + "_py" for dep in deps],
                protos=protos,
                srcs_version = python_src_version,
                **kwargs
            )
        else:
            fail("Unknown language chosen: " + lang)
        
