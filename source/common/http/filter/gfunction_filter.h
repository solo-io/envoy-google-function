#pragma once

#include <map>
#include <string>

#include "envoy/http/metadata_accessor.h"
#include "envoy/upstream/cluster_manager.h"

#include "common/common/logger.h"
#include "common/http/filter/google_authenticator.h"
#include "common/http/filter/solo_logger.h"

#include "server/config/network/http_connection_manager.h"

namespace Envoy {
namespace Http {

struct Function {
  std::string func_name_;
  std::string hostname_;
  std::string region_;
  std::string project_;
};

typedef std::map<std::string, Function> ClusterFunctionMap;

class GfunctionFilter
    : public Envoy::Http::StreamFilter,
      public FunctionalFilter ,
      public Envoy::Logger::Loggable<Envoy::Logger::Id::filter> {
public:
  GfunctionFilter(Envoy::Upstream::ClusterManager &cm, CallbackerSharedPtr cb);
  ~GfunctionFilter();

  // Http::StreamFilterBase
  void onDestroy() override;

  // Http::StreamDecoderFilter
  Envoy::Http::FilterHeadersStatus
  decodeHeaders(Envoy::Http::HeaderMap &headers, bool) override;
  Envoy::Http::FilterDataStatus decodeData(Envoy::Buffer::Instance &,
                                           bool) override;
  Envoy::Http::FilterTrailersStatus
  decodeTrailers(Envoy::Http::HeaderMap &) override;
  void setDecoderFilterCallbacks(
      Envoy::Http::StreamDecoderFilterCallbacks &callbacks) override;

  // Http::StreamEncoderFilter
  Envoy::Http::FilterHeadersStatus
  encodeHeaders(Envoy::Http::HeaderMap &headers, bool) override;
  Envoy::Http::FilterDataStatus encodeData(Envoy::Buffer::Instance &,
                                           bool) override;
  Envoy::Http::FilterTrailersStatus
  encodeTrailers(Envoy::Http::HeaderMap &) override;
  void setEncoderFilterCallbacks(
      Envoy::Http::StreamEncoderFilterCallbacks &callbacks) override;
  FilterHeadersStatus encode100ContinueHeaders(HeaderMap &) override {
    return Http::FilterHeadersStatus::Continue;
  }
  
  // Http::FunctionRetriever
  bool retrieveFunction(const MetadataAccessor &meta_accessor) override;

private:
  Envoy::Http::StreamDecoderFilterCallbacks *decoder_callbacks_;
  Envoy::Http::StreamEncoderFilterCallbacks *encoder_callbacks_;
  
  Optional<const std::string *> host_;
  Optional<const std::string *> path_;
  
  void Gfunctionfy();



  Envoy::Http::HeaderMap *request_headers_{};
  Envoy::Http::HeaderMap *tracing_headers_{};
  
  bool active_;

  bool tracingEnabled_;

  CloudCollector collector_;
};

} // namespace Http
} // namespace Envoy
