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
                                 CallbackerSharedPtr cb, std::string access_key,
                                 std::string secret_key,
                                 ClusterFunctionMap functions)
    : functions_(std::move(functions)), active_(false), tracingEnabled_(false),
      googleAuthenticator_(std::move(access_key), std::move(secret_key),
                           std::string("Gfunction")),
      collector_(cm, cb) {}

GfunctionFilter::~GfunctionFilter() {}

void GfunctionFilter::onDestroy() {}

std::string GfunctionFilter::functionUrlPath() {
  std::stringstream val;
  val << "/" << currentFunction_.func_name_;
  return val.str();
}

std::string GfunctionFilter::functionHostName() {
  std::stringstream val;
  val << currentFunction_.region_ << "-" << currentFunction_.project_ << "."
      << currentFunction_.hostname_;
  return val.str();
}

Envoy::Http::FilterHeadersStatus
GfunctionFilter::decodeHeaders(Envoy::Http::HeaderMap &headers,
                               bool end_stream) {

  const Envoy::Router::RouteEntry *routeEntry =
      decoder_callbacks_->route()->routeEntry();

  if (routeEntry == nullptr) {
    return Envoy::Http::FilterHeadersStatus::Continue;
  }

  const std::string &cluster_name = routeEntry->clusterName();
  ClusterFunctionMap::iterator currentFunction = functions_.find(cluster_name);
  if (currentFunction == functions_.end()) {
    return Envoy::Http::FilterHeadersStatus::Continue;
  }

  active_ = true;
  currentFunction_ = currentFunction->second;

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

  logHeaders(headers);

  ENVOY_LOG(debug, "GFUNCTION: decodeHeaders called end = {}", end_stream);

  return Envoy::Http::FilterHeadersStatus::Continue;
}

Envoy::Http::FilterDataStatus
GfunctionFilter::decodeData(Envoy::Buffer::Instance &, bool) {
  return Envoy::Http::FilterDataStatus::Continue;
}

void GfunctionFilter::Gfunctionfy() {

  request_headers_->insertMethod().value().setReference(
      Envoy::Http::Headers::get().MethodValues.Post);

  request_headers_->insertPath().value(functionUrlPath());
  request_headers_->insertHost().value(functionHostName());
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
                               bool end_stream) {
  const Envoy::Router::RouteEntry *routeEntry =
      encoder_callbacks_->route()->routeEntry();
  ENVOY_LOG(debug, "GFUNCTION: encodeHeaders called end = {}", end_stream);
  if (routeEntry == nullptr) {
    return Envoy::Http::FilterHeadersStatus::Continue;
  }

  const std::string &cluster_name = routeEntry->clusterName();
  ClusterFunctionMap::iterator currentFunction = functions_.find(cluster_name);
  if (currentFunction == functions_.end()) {
    return Envoy::Http::FilterHeadersStatus::Continue;
  }

  active_ = true;
  currentFunction_ = currentFunction->second;

  logHeaders(headers);
  request_headers_ = &headers;

  if (tracingEnabled_) {
    ENVOY_LOG(info, "GFUNCTION: Storing cloud tracing info");
    const Envoy::Http::HeaderEntry *hdr =
        headers.get(Envoy::Http::LowerCaseString("function-execution-id"));
    if (hdr != nullptr) {
      CloudCollector::RequestInfo info;
      info.function_name_ = currentFunction_.func_name_;
      info.region_ = currentFunction_.region_;
      info.project_ = currentFunction_.project_;
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

void GfunctionFilter::logHeaders(Envoy::Http::HeaderMap &headers) {
  // Print all headers - DEBUG
  headers.iterate(
      [](const Envoy::Http::HeaderEntry &header,
         void *) -> Envoy::Http::HeaderMap::Iterate {
        ENVOY_LOG(debug, "GFUNCTION: '{}':'{}'", header.key().c_str(),
                  header.value().c_str());
        return Envoy::Http::HeaderMap::Iterate::Continue;
      },
      this);
}

} // namespace Http
} // namespace Envoy
