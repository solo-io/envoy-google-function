#include "common/http/filter/gfunction_filter.h"

#include <algorithm>
#include <list>
#include <string>
#include <vector>

#include "envoy/http/header_map.h"

#include "common/common/empty_string.h"
#include "common/common/hex.h"
#include "common/common/utility.h"

#include "server/config/network/http_connection_manager.h"

namespace Envoy {
namespace Http {

GfunctionFilter::GfunctionFilter(Envoy::Upstream::ClusterManager &cm,
                                 CallbackerSharedPtr cb)
    : tracingEnabled_(false), collector_(cm, cb) {}

GfunctionFilter::~GfunctionFilter() {}

void GfunctionFilter::onDestroy() {}

Optional<const Protobuf::Value *>
maybevalue(const Protobuf::Struct &spec,
                                 const std::string &key) {
  const auto &fields = spec.fields();
  const auto fields_it = fields.find(key);
  if (fields_it == fields.end()) {
    return {};
  }

  const auto &value = fields_it->second;
  return &value;
}

// TODO(yuval-k): this is here only to see the e2e working; move to common asap.
Optional<const std::string *> nonEmptyStringValue(const ProtobufWkt::Struct &spec,
                                               const std::string &key) {

  Optional<const Protobuf::Value *> maybe_value = maybevalue(spec, key);
  if (!maybe_value.valid()) {
    return {};
  }
  const auto &value = *maybe_value.value();
  if (value.kind_case() != ProtobufWkt::Value::kStringValue) {
    return {};
  }

  const auto &string_value = value.string_value();
  if (string_value.empty()) {
    return {};
  }

  return Optional<const std::string *>(&string_value);
}

bool GfunctionFilter::retrieveFunction(const MetadataAccessor &meta_accessor) {
  
  
  Optional<const ProtobufWkt::Struct *> maybe_function_spec = meta_accessor.getFunctionSpec();
  if (!maybe_function_spec.valid()) {
    return false;
  }
  const ProtobufWkt::Struct &function_spec = *maybe_function_spec.value();

  host_ = nonEmptyStringValue(function_spec, "host");
  path_ = nonEmptyStringValue(function_spec, "path");

  return host_.valid() && path_.valid();
}

Envoy::Http::FilterHeadersStatus
GfunctionFilter::decodeHeaders(Envoy::Http::HeaderMap &headers,
                               bool) {

  request_headers_ = &headers;
  Gfunctionfy();

  // Check if tracing is enabled
  const Envoy::Http::HeaderEntry *p = headers.XB3TraceId();
  if (p != NULL) {
    tracingEnabled_ = true;
    tracing_headers_ = &headers;
  } else {
    tracingEnabled_ = false;
  }

  return Envoy::Http::FilterHeadersStatus::Continue;
}

Envoy::Http::FilterDataStatus
GfunctionFilter::decodeData(Envoy::Buffer::Instance &, bool) {
  return Envoy::Http::FilterDataStatus::Continue;
}

void GfunctionFilter::Gfunctionfy() {

  request_headers_->insertMethod().value().setReference(
      Envoy::Http::Headers::get().MethodValues.Post);

  request_headers_->insertPath().value().setReference(*path_.value());
  request_headers_->insertHost().value().setReference(*host_.value());
}

Envoy::Http::FilterTrailersStatus
GfunctionFilter::decodeTrailers(Envoy::Http::HeaderMap &) {
  return Envoy::Http::FilterTrailersStatus::Continue;
}

void GfunctionFilter::setDecoderFilterCallbacks(
    Envoy::Http::StreamDecoderFilterCallbacks &callbacks) {
  decoder_callbacks_ = &callbacks;
}

Envoy::Http::FilterHeadersStatus
GfunctionFilter::encodeHeaders(Envoy::Http::HeaderMap &headers,
                               bool) {

  request_headers_ = &headers;

  if (tracingEnabled_) {
    const Envoy::Http::HeaderEntry *hdr =
        headers.get(Envoy::Http::LowerCaseString("function-execution-id"));
    if (hdr != nullptr) {
      CloudCollector::RequestInfo info;
      info.function_name_ = *path_.value();
      info.region_ = *host_.value();
      info.project_ = *host_.value();
      info.provider_ = "google";
      info.request_id_ = hdr->value().c_str();
      collector_.storeRequestInfo(info, tracing_headers_);
    }
  } else {
    ENVOY_LOG(info, "GFUNCTION: Not storing cloud tracing info");
  }
  return Envoy::Http::FilterHeadersStatus::Continue;
}

Envoy::Http::FilterDataStatus
GfunctionFilter::encodeData(Envoy::Buffer::Instance &, bool) {
  return Envoy::Http::FilterDataStatus::Continue;
}

Envoy::Http::FilterTrailersStatus
GfunctionFilter::encodeTrailers(Envoy::Http::HeaderMap &) {
  return Envoy::Http::FilterTrailersStatus::Continue;
}

void GfunctionFilter::setEncoderFilterCallbacks(
    Envoy::Http::StreamEncoderFilterCallbacks &callbacks) {
  encoder_callbacks_ = &callbacks;
}


} // namespace Http
} // namespace Envoy
