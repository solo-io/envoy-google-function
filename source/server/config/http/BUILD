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
    name = "gfunction_filter_config",
    srcs = ["gfunction_filter_config.cc"],
    repository = "@envoy",
    deps = [
        "//source/common/config:gfunction_well_known_names_lib",
        "//source/common/http/filter:gfunction_filter_lib",
        "@envoy//include/envoy/registry",
        "@envoy//include/envoy/server:filter_config_interface",
        "@envoy//source/extensions/filters/http/common:empty_http_filter_config_lib",
    ],
)
