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

class GfunctionFilter : public StreamFilter,
                        public FunctionalFilter,
                        public Logger::Loggable<Logger::Id::filter> {
public:
  GfunctionFilter(Upstream::ClusterManager &cm, CallbackerSharedPtr cb);
  ~GfunctionFilter();

  // Http::StreamFilterBase
  void onDestroy() override;

  // Http::StreamDecoderFilter
  FilterHeadersStatus decodeHeaders(HeaderMap &headers, bool) override;
  FilterDataStatus decodeData(Buffer::Instance &, bool) override;
  FilterTrailersStatus decodeTrailers(HeaderMap &) override;
  void
  setDecoderFilterCallbacks(StreamDecoderFilterCallbacks &callbacks) override;

  // Http::StreamEncoderFilter
  FilterHeadersStatus encodeHeaders(HeaderMap &headers, bool) override;
  FilterDataStatus encodeData(Buffer::Instance &, bool) override;
  FilterTrailersStatus encodeTrailers(HeaderMap &) override;
  void
  setEncoderFilterCallbacks(StreamEncoderFilterCallbacks &callbacks) override;

  FilterHeadersStatus encode100ContinueHeaders(HeaderMap &) override {
    return FilterHeadersStatus::Continue;
  }

  // Http::FunctionRetriever
  bool retrieveFunction(const MetadataAccessor &meta_accessor) override;

private:
  StreamDecoderFilterCallbacks *decoder_callbacks_;
  StreamEncoderFilterCallbacks *encoder_callbacks_;

  Optional<const std::string *> host_;
  Optional<const std::string *> path_;

  void Gfunctionfy();

  HeaderMap *request_headers_{};
  HeaderMap *tracing_headers_{};

  bool active_;

  bool tracingEnabled_;

  CloudCollector collector_;
};

} // namespace Http
} // namespace Envoy
