#include <string>
#include <algorithm>
#include <vector>
#include <list>

#include "gfunction_filter.h"

#include "server/config/network/http_connection_manager.h"

#include "envoy/http/header_map.h"

#include "common/common/hex.h"
#include "common/common/empty_string.h"
#include "common/common/utility.h"

namespace Solo {
namespace Gfunction {

GfunctionFilter::GfunctionFilter(std::string access_key, std::string secret_key, ClusterFunctionMap functions) : 
  functions_(std::move(functions)),
  active_(false), 
  googleAuthenticator_(std::move(access_key), std::move(secret_key), std::string("Gfunction")) {
}

GfunctionFilter::~GfunctionFilter() {}

void GfunctionFilter::onDestroy() {}

std::string GfunctionFilter::functionUrlPath() {
  std::stringstream val;
  val << "/"<< currentFunction_.func_name_;
  return val.str();
}

std::string GfunctionFilter::functionHostName() {
  std::stringstream val;
  val << currentFunction_.region_ << "-" << currentFunction_.project_ << "." << currentFunction_.hostname_;
  return val.str();
}

Envoy::Http::FilterHeadersStatus GfunctionFilter::decodeHeaders(Envoy::Http::HeaderMap& headers, bool end_stream) {
  
  ENVOY_LOG(debug, "GFUNCTION START: decodeHeaders called, end = {}", end_stream);

  const Envoy::Router::RouteEntry* routeEntry = decoder_callbacks_->route()->routeEntry();

  if (routeEntry == nullptr) {
    return Envoy::Http::FilterHeadersStatus::Continue;    
  }

  const std::string& cluster_name = routeEntry->clusterName();
  ClusterFunctionMap::iterator currentFunction = functions_.find(cluster_name);
  if (currentFunction == functions_.end()){
    return Envoy::Http::FilterHeadersStatus::Continue;    
  }

  active_ = true;
  currentFunction_ = currentFunction->second;

  headers.insertMethod().value().setReference(Envoy::Http::Headers::get().MethodValues.Post);
  
//  headers.removeContentLength();  
  headers.insertPath().value(functionUrlPath());
  request_headers_ = &headers;
  
  ENVOY_LOG(debug, "GFUNCTION END: decodeHeaders called end = {}", end_stream);
  
  return Envoy::Http::FilterHeadersStatus::StopIteration;
}

Envoy::Http::FilterDataStatus GfunctionFilter::decodeData(Envoy::Buffer::Instance& data, bool end_stream) {

  if (!active_) {
    return Envoy::Http::FilterDataStatus::Continue;    
  }
  // calc hash of data
  ENVOY_LOG(debug, "GFUNCTION START: decodeData called end = {} data = {}", end_stream, data.length());

  if (end_stream) {

    Gfunctionfy();
    request_headers_ = nullptr;
    active_ = false;
    // add header ?!
    // get stream id
    ENVOY_LOG(debug, "GFUNCTION END_STREAM: decodeData called end = {} data = {}", end_stream, data.length());
    return Envoy::Http::FilterDataStatus::Continue;
  }
  ENVOY_LOG(debug, "GFUNCTION CONTINUE: decodeData called end = {} data = {}", end_stream, data.length());
  return Envoy::Http::FilterDataStatus::StopIterationAndBuffer;
}

void GfunctionFilter::Gfunctionfy() {
  ENVOY_LOG(debug, "GFUNCTION START: Gfunctionfy");

  std::list<Envoy::Http::LowerCaseString> headers;
  
  headers.push_back(Envoy::Http::LowerCaseString("host"));
  request_headers_->insertHost().value(functionHostName());

  headers.push_back(Envoy::Http::LowerCaseString("content-type"));
}

Envoy::Http::FilterTrailersStatus GfunctionFilter::decodeTrailers(Envoy::Http::HeaderMap&) {
  if (!active_) {
    return Envoy::Http::FilterTrailersStatus::Continue;    
  }

  Gfunctionfy();
  
  return Envoy::Http::FilterTrailersStatus::Continue;
}

void GfunctionFilter::setDecoderFilterCallbacks(Envoy::Http::StreamDecoderFilterCallbacks& callbacks) {
  decoder_callbacks_ = &callbacks;
}

} // Gfunction
} // Solo
