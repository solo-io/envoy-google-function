# Envoy Google Cloud Functions filter

This project links a [Google Cloud Functions](https://cloud.google.com/functions/) HTTP filter with
the [Envoy](https://www.envoyproxy.io/) binary.
A new filter `io.solo.gcloudfunc` which redirects requests to Google Cloud Functions is introduced.

## Building

To build the Envoy static binary:

```
$ bazel build //:envoy
```

## Testing

To run the all tests:

```
$ bazel test //test/...
```

To run the all tests in debug mode:

```
$ bazel test //test/... -c dbg
```

To run integration tests using a clang build:

```
$ CXX=clang++-5.0 CC=clang-5.0  bazel test -c dbg --config=clang-tsan //test/integration:gfunction_filter_integration_test
```

## E2E

The e2e test requires [installing Google Cloud SDK](https://cloud.google.com/sdk/install).

To run the e2e test:

```
$ bazel test //e2e/...
```
