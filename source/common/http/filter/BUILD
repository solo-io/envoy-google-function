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
    name = "solo_logger_lib",
    srcs = ["solo_logger.cc"],
    hdrs = ["solo_logger.h"],
    repository = "@envoy",
    deps = [
        "@envoy//source/exe:envoy_common_lib",
    ],
)

envoy_cc_library(
    name = "google_authenticator_lib",
    srcs = ["google_authenticator.cc"],
    hdrs = ["google_authenticator.h"],
    repository = "@envoy",
    deps = [
        "@envoy//source/exe:envoy_common_lib",
    ],
)

envoy_cc_library(
    name = "gfunction_filter_lib",
    srcs = ["gfunction_filter.cc"],
    hdrs = ["gfunction_filter.h"],
    repository = "@envoy",
    deps = [
        ":google_authenticator_lib",
        ":solo_logger_lib",
        "//source/common/config:gfunction_well_known_names_lib",
        "@envoy//source/exe:envoy_common_lib",
        "@solo_envoy_common//source/common/http:functional_stream_decoder_base_lib",
    ],
)
