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

GfunctionFilter::GfunctionFilter(
    Envoy::Upstream::ClusterManager& cm, 
    std::string access_key, 
    std::string secret_key, 
    ClusterFunctionMap functions) :
  functions_(std::move(functions)),
  active_(false), 
  tracingEnabled_(false),
  googleAuthenticator_(std::move(access_key), std::move(secret_key), std::string("Gfunction")), 
  collector_(cm) {
}

GfunctionFilter::~GfunctionFilter() {}

void GfunctionFilter::onDestroy() {
  collector_.abortRequest();
}

std::string GfunctionFilter::functionUrlPath() {
  std::stringstream val;
  val << "/" << currentFunction_.func_name_;
  return val.str();
}

std::string GfunctionFilter::functionHostName() {
  std::stringstream val;
  val << currentFunction_.region_ << "-" << currentFunction_.project_ << "." << currentFunction_.hostname_;
  return val.str();
}

Envoy::Http::FilterHeadersStatus GfunctionFilter::decodeHeaders(Envoy::Http::HeaderMap& headers, bool end_stream) {
  
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
  headers.insertHost().value(functionHostName());

  // Check if tracing is enabled
  const Envoy::Http::HeaderEntry* p = headers.XB3TraceId();
  if(p != NULL) {
    ENVOY_LOG(debug, "GFUNCTION: Tracing is ON");
    tracingEnabled_ = true;
    tracing_headers_ = &headers;
  }
  else {
    ENVOY_LOG(debug, "GFUNCTION: Tracing is OFF");
    tracingEnabled_ = false;
  }

  request_headers_ = &headers;

  logHeaders(headers);

  ENVOY_LOG(debug, "GFUNCTION: decodeHeaders called end = {}", end_stream);
  
  return Envoy::Http::FilterHeadersStatus::StopIteration;
}

Envoy::Http::FilterDataStatus GfunctionFilter::decodeData(Envoy::Buffer::Instance& data, bool end_stream) {

  if (!active_) {
    return Envoy::Http::FilterDataStatus::Continue;    
  }
  // calc hash of data
  ENVOY_LOG(debug, "GFUNCTION: decodeData called end = {} data = {}", end_stream, data.length());

  if (end_stream) {
    Gfunctionfy();
    request_headers_ = nullptr;
    active_ = false;
    return Envoy::Http::FilterDataStatus::Continue;
  }
  return Envoy::Http::FilterDataStatus::StopIterationAndBuffer;
}

void GfunctionFilter::Gfunctionfy() {
  std::list<Envoy::Http::LowerCaseString> headers;
  
  headers.push_back(Envoy::Http::LowerCaseString("host"));
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

Envoy::Http::FilterHeadersStatus GfunctionFilter::encodeHeaders(Envoy::Http::HeaderMap& headers, bool end_stream) {
  const Envoy::Router::RouteEntry* routeEntry = encoder_callbacks_->route()->routeEntry();
  ENVOY_LOG(debug, "GFUNCTION: encodeHeaders called end = {}", end_stream);
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
  
  logHeaders(headers);
  request_headers_ = &headers;

  //tracingEnabled_ = true;
  if(tracingEnabled_) {
    ENVOY_LOG(info, "GFUNCTION: Storing cloud tracing info");
    const Envoy::Http::HeaderEntry* hdr = headers.get(
      Envoy::Http::LowerCaseString("function-execution-id"));
    if(hdr != nullptr) {
      solo::logger::RequestInfo info;
      info.function_name_ = currentFunction_.func_name_;
      info.region_ = currentFunction_.region_;
      info.project_ = currentFunction_.project_;
      info.provider_ = "google";
      info.request_id_ = hdr->value().c_str();
      collector_.storeRequestInfo(info, tracing_headers_);
    }
  }
  else {
    ENVOY_LOG(info, "GFUNCTION: Not storing cloud tracing info");
  }
  return Envoy::Http::FilterHeadersStatus::StopIteration;
}

Envoy::Http::FilterDataStatus GfunctionFilter::encodeData(Envoy::Buffer::Instance&, bool end_stream) {
  ENVOY_LOG(debug, "GFUNCTION: encodeData called end = {}", end_stream);
  return Envoy::Http::FilterDataStatus::Continue;
  /*
  if (!active_) {
    return Envoy::Http::FilterDataStatus::Continue;    
  }
  if (end_stream) {
    request_headers_ = nullptr;
    active_ = false;
    // add header ?!
    // get stream id
    return Envoy::Http::FilterDataStatus::Continue;
  }
  return Envoy::Http::FilterDataStatus::StopIterationAndBuffer;
  */
}

Envoy::Http::FilterTrailersStatus GfunctionFilter::encodeTrailers(Envoy::Http::HeaderMap&) {
  return Envoy::Http::FilterTrailersStatus::Continue; 
}

void GfunctionFilter::setEncoderFilterCallbacks(Envoy::Http::StreamEncoderFilterCallbacks& callbacks) {
  encoder_callbacks_ = &callbacks;
}

void GfunctionFilter::logHeaders(Envoy::Http::HeaderMap& headers) {
 // Print all headers - DEBUG
  headers.iterate(
      [](const Envoy::Http::HeaderEntry& header, void* ) -> Envoy::Http::HeaderMap::Iterate {
        ENVOY_LOG(debug, "  '{}':'{}'",
                         header.key().c_str(), header.value().c_str());
        return Envoy::Http::HeaderMap::Iterate::Continue;
      },
      this);
}

} // Gfunction
} // Solo
