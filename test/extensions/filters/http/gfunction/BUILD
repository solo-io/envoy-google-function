licenses(["notice"])  # Apache 2

load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_binary",
    "envoy_cc_library",
    "envoy_cc_test",
    "envoy_package",
)

envoy_package()

envoy_cc_test(
    name = "gfunction_filter_integration_test",
    srcs = ["gfunction_filter_integration_test.cc"],
    data = [":envoy-test.conf"],
    repository = "@envoy",
    deps = [
        "//source/extensions/filters/http/gfunction:gfunction_filter_config",
        "@envoy//test/integration:http_integration_lib",
        "@envoy//test/integration:integration_lib",
    ],
)
