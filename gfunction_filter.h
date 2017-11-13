#pragma once

#include <string>
#include <map>

#include "server/config/network/http_connection_manager.h"
#include "envoy/upstream/cluster_manager.h"

#include "common/common/logger.h"

#include "google_authenticator.h"

namespace Solo {
namespace Gfunction {

struct Function {
  std::string func_name_;
  std::string hostname_;
  std::string region_;
  std::string project_;
};

typedef std::map<std::string, Function> ClusterFunctionMap;


class GfunctionFilter : public Envoy::Http::StreamDecoderFilter,  public Envoy::Logger::Loggable<Envoy::Logger::Id::filter> {
public:
  GfunctionFilter(std::string access_key, std::string secret_key, ClusterFunctionMap functions);
  ~GfunctionFilter();

  // Http::StreamFilterBase
  void onDestroy() override;

  // Http::StreamDecoderFilter
  Envoy::Http::FilterHeadersStatus decodeHeaders(Envoy::Http::HeaderMap& headers, bool) override;
  Envoy::Http::FilterDataStatus decodeData(Envoy::Buffer::Instance&, bool) override;
  Envoy::Http::FilterTrailersStatus decodeTrailers(Envoy::Http::HeaderMap&) override;
  void setDecoderFilterCallbacks(Envoy::Http::StreamDecoderFilterCallbacks& callbacks) override;

private:
  Envoy::Http::StreamDecoderFilterCallbacks* decoder_callbacks_;
  ClusterFunctionMap functions_;
  Function currentFunction_;
  void Gfunctionfy();
  std::string functionUrlPath();
  std::string functionHostName();

  Envoy::Http::HeaderMap* request_headers_{};
  bool active_;
  GoogleAuthenticator googleAuthenticator_;
};

} // Gfunction
} // Solo