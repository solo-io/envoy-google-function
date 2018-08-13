licenses(["notice"])  # Apache 2

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_binary",
    "envoy_cc_library",
    "envoy_cc_test",
    "envoy_package",
)

envoy_package()

envoy_cc_library(
    name = "gfunction_filter_lib",
    srcs = ["gfunction_filter.cc"],
    hdrs = ["gfunction_filter.h"],
    repository = "@envoy",
    deps = [
        "//source/common/config:gfunction_well_known_names_lib",
        "@envoy//include/envoy/http:filter_interface",
        "@envoy//source/common/common:hex_lib",
        "@envoy//source/common/http:headers_lib",
        "@solo_envoy_common//source/common/config:solo_metadata_lib",
        "@solo_envoy_common//source/common/http:functional_stream_decoder_base_lib",
    ],
)

envoy_cc_library(
    name = "gfunction_filter_config",
    srcs = ["gfunction_filter_config.cc"],
    repository = "@envoy",
    deps = [
        ":gfunction_filter_lib",
        "//source/common/config:gfunction_well_known_names_lib",
        "@envoy//include/envoy/registry",
        "@envoy//include/envoy/server:filter_config_interface",
        "@envoy//source/extensions/filters/http/common:empty_http_filter_config_lib",
    ],
)
