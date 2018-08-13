#include "extensions/filters/http/gfunction/gfunction_filter.h"

#include <algorithm>
#include <list>
#include <string>
#include <vector>

#include "envoy/http/header_map.h"

#include "common/common/empty_string.h"
#include "common/common/hex.h"
#include "common/common/utility.h"
#include "common/config/gfunction_well_known_names.h"
#include "common/config/solo_metadata.h"
#include "common/http/headers.h"

namespace Envoy {
namespace Http {

using Config::SoloMetadata;

GfunctionFilter::GfunctionFilter() {}

GfunctionFilter::~GfunctionFilter() {}

void GfunctionFilter::onDestroy() {}

bool GfunctionFilter::retrieveFunction(const MetadataAccessor &meta_accessor) {

  absl::optional<const ProtobufWkt::Struct *> maybe_function_spec =
      meta_accessor.getFunctionSpec();
  if (!maybe_function_spec.has_value()) {
    return false;
  }
  const ProtobufWkt::Struct &function_spec = *maybe_function_spec.value();

  host_ = SoloMetadata::nonEmptyStringValue(
      function_spec, Config::MetadataGFunctionKeys::get().HOST);
  path_ = SoloMetadata::nonEmptyStringValue(
      function_spec, Config::MetadataGFunctionKeys::get().PATH);

  return host_.has_value() && path_.has_value();
}

FilterHeadersStatus GfunctionFilter::decodeHeaders(HeaderMap &headers, bool) {
  Gfunctionfy(headers);
  return FilterHeadersStatus::Continue;
}

FilterDataStatus GfunctionFilter::decodeData(Buffer::Instance &, bool) {
  return FilterDataStatus::Continue;
}

void GfunctionFilter::Gfunctionfy(HeaderMap &headers) {

  headers.insertMethod().value().setReference(Headers::get().MethodValues.Post);

  headers.insertPath().value().setReference(*path_.value());
  headers.insertHost().value().setReference(*host_.value());
}

FilterTrailersStatus GfunctionFilter::decodeTrailers(HeaderMap &) {
  return FilterTrailersStatus::Continue;
}

void GfunctionFilter::setDecoderFilterCallbacks(
    StreamDecoderFilterCallbacks &callbacks) {
  UNREFERENCED_PARAMETER(callbacks);
}

} // namespace Http
} // namespace Envoy
