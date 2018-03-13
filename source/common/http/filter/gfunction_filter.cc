#include "common/http/filter/gfunction_filter.h"

#include <algorithm>
#include <list>
#include <string>
#include <vector>

#include "envoy/http/header_map.h"

#include "common/common/empty_string.h"
#include "common/common/hex.h"
#include "common/common/utility.h"
#include "common/config/gfunction_well_known_names.h"

#include "server/config/network/http_connection_manager.h"

namespace Envoy {
namespace Http {

GfunctionFilter::GfunctionFilter() {}

GfunctionFilter::~GfunctionFilter() {}

void GfunctionFilter::onDestroy() {}

Optional<const Protobuf::Value *> maybevalue(const Protobuf::Struct &spec,
                                             const std::string &key) {
  const auto &fields = spec.fields();
  const auto fields_it = fields.find(key);
  if (fields_it == fields.end()) {
    return {};
  }

  const auto &value = fields_it->second;
  return &value;
}

Optional<const std::string *>
nonEmptyStringValue(const ProtobufWkt::Struct &spec, const std::string &key) {

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

  Optional<const ProtobufWkt::Struct *> maybe_function_spec =
      meta_accessor.getFunctionSpec();
  if (!maybe_function_spec.valid()) {
    return false;
  }
  const ProtobufWkt::Struct &function_spec = *maybe_function_spec.value();

  host_ = nonEmptyStringValue(function_spec,
                              Config::MetadataGFunctionKeys::get().HOST);
  path_ = nonEmptyStringValue(function_spec,
                              Config::MetadataGFunctionKeys::get().PATH);

  return host_.valid() && path_.valid();
}

FilterHeadersStatus GfunctionFilter::decodeHeaders(HeaderMap &headers, bool) {
  Gfunctionfy(headers);
  return FilterHeadersStatus::Continue;
}

FilterDataStatus GfunctionFilter::decodeData(Buffer::Instance &, bool) {
  return FilterDataStatus::Continue;
}

void GfunctionFilter::Gfunctionfy(HeaderMap &headers) {

  headers.insertMethod().value().setReference(
      Headers::get().MethodValues.Post);

  headers.insertPath().value().setReference(*path_.value());
  headers.insertHost().value().setReference(*host_.value());
}

FilterTrailersStatus GfunctionFilter::decodeTrailers(HeaderMap &) {
  return FilterTrailersStatus::Continue;
}

void GfunctionFilter::setDecoderFilterCallbacks(
    StreamDecoderFilterCallbacks &callbacks) {
  decoder_callbacks_ = &callbacks;
}

} // namespace Http
} // namespace Envoy
