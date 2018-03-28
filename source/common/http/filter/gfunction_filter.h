#pragma once

#include <map>
#include <string>

#include "envoy/http/metadata_accessor.h"
#include "envoy/upstream/cluster_manager.h"

#include "common/common/logger.h"

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

/**
 * Since currently all http cloud functions must have a public endpoint, no auth
 * is needed. This filter just changes the headers to match the function.
 */
class GfunctionFilter : public StreamDecoderFilter,
                        public FunctionalFilter,
                        public Logger::Loggable<Logger::Id::filter> {
public:
  GfunctionFilter();
  ~GfunctionFilter();

  // Http::StreamFilterBase
  void onDestroy() override;

  // Http::StreamDecoderFilter
  FilterHeadersStatus decodeHeaders(HeaderMap &headers, bool) override;
  FilterDataStatus decodeData(Buffer::Instance &, bool) override;
  FilterTrailersStatus decodeTrailers(HeaderMap &) override;
  void
  setDecoderFilterCallbacks(StreamDecoderFilterCallbacks &callbacks) override;

  // Http::FunctionRetriever
  bool retrieveFunction(const MetadataAccessor &meta_accessor) override;

private:
  StreamDecoderFilterCallbacks *decoder_callbacks_;

  Optional<const std::string *> host_;
  Optional<const std::string *> path_;

  void Gfunctionfy(HeaderMap &headers);
};

} // namespace Http
} // namespace Envoy
