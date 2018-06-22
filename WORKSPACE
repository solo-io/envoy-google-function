workspace(name="google_functions")

# Use skylark for native Git.
load('@bazel_tools//tools/build_defs/repo:git.bzl', 'git_repository')

ENVOY_SHA = "2b216ca50c7cd04e8736cb44b39fbdedc00c86b3"  # Jun 14, 2018 (Correct AddrFaily to AddrFamily. (#3636))

http_archive(
    name = "envoy",
    strip_prefix = "envoy-" + ENVOY_SHA,
    url = "https://github.com/envoyproxy/envoy/archive/" + ENVOY_SHA + ".zip",
)

ENVOY_COMMON_SHA = "92f11092c725026a81ea8634a2c4df27705d072d"  # May 16, 2018 (Upgrade Envoy)

http_archive(
    name = "solo_envoy_common",
    strip_prefix = "envoy-common-" + ENVOY_COMMON_SHA,
    url = "https://github.com/solo-io/envoy-common/archive/" + ENVOY_COMMON_SHA + ".zip",
)

load("@envoy//bazel:repositories.bzl", "envoy_dependencies")
load("@envoy//bazel:cc_configure.bzl", "cc_configure")

envoy_dependencies()

cc_configure()

load("@envoy_api//bazel:repositories.bzl", "api_dependencies")
api_dependencies()

load("@io_bazel_rules_go//go:def.bzl", "go_rules_dependencies", "go_register_toolchains")
load("@com_lyft_protoc_gen_validate//bazel:go_proto_library.bzl", "go_proto_repositories")
go_proto_repositories(shared=0)
go_rules_dependencies()
go_register_toolchains()
load("@io_bazel_rules_go//proto:def.bzl", "proto_register_toolchains")
proto_register_toolchains()
