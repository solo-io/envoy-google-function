#pragma once

#include <string>
#include <map>

#include "server/config/network/http_connection_manager.h"
#include "envoy/upstream/cluster_manager.h"

#include "common/common/logger.h"

#include "google_authenticator.h"
#include "solo_logger.h"

namespace Solo {
namespace Gfunction {

struct Function {
  std::string func_name_;
  std::string hostname_;
  std::string region_;
  std::string project_;
};

typedef std::map<std::string, Function> ClusterFunctionMap;

class GfunctionFilter : 
        public Envoy::Http::StreamFilter, 
        public Envoy::Logger::Loggable<Envoy::Logger::Id::filter> {
public:
  GfunctionFilter(Envoy::Upstream::ClusterManager& cm, std::string access_key, std::string secret_key, ClusterFunctionMap functions);
  ~GfunctionFilter();

  // Http::StreamFilterBase
  void onDestroy() override;

  // Http::StreamDecoderFilter
  Envoy::Http::FilterHeadersStatus decodeHeaders(Envoy::Http::HeaderMap& headers, bool) override;
  Envoy::Http::FilterDataStatus decodeData(Envoy::Buffer::Instance&, bool) override;
  Envoy::Http::FilterTrailersStatus decodeTrailers(Envoy::Http::HeaderMap&) override;
  void setDecoderFilterCallbacks(Envoy::Http::StreamDecoderFilterCallbacks& callbacks) override;

  // Http::StreamEncoderFilter
  Envoy::Http::FilterHeadersStatus encodeHeaders(Envoy::Http::HeaderMap& headers, bool) override;
  Envoy::Http::FilterDataStatus encodeData(Envoy::Buffer::Instance&, bool) override;
  Envoy::Http::FilterTrailersStatus encodeTrailers(Envoy::Http::HeaderMap&) override;
  void setEncoderFilterCallbacks(Envoy::Http::StreamEncoderFilterCallbacks& callbacks) override;


private:
  Envoy::Http::StreamDecoderFilterCallbacks* decoder_callbacks_;
  Envoy::Http::StreamEncoderFilterCallbacks* encoder_callbacks_;
  ClusterFunctionMap functions_;
  Function currentFunction_;
  void Gfunctionfy();
  void logHeaders(Envoy::Http::HeaderMap&);
  std::string functionUrlPath();
  std::string functionHostName();

  Envoy::Http::HeaderMap* request_headers_{};
  bool active_;
  bool tracingEnabled_;
  GoogleAuthenticator googleAuthenticator_;

  solo::logger::CloudCollector collector_;
};

} // Gfunction
} // Solo