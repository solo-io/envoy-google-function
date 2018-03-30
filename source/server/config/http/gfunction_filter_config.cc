#include <string>

#include "envoy/registry/registry.h"

#include "common/config/gfunction_well_known_names.h"
#include "common/http/filter/gfunction_filter.h"
#include "common/http/functional_stream_decoder_base.h"

#include "server/config/http/empty_http_filter_config.h"

namespace Envoy {
namespace Server {
namespace Configuration {

typedef Http::FunctionalFilterMixin<Http::GfunctionFilter> MixedGFunctionFilter;

class GfunctionFilterFactory : public EmptyHttpFilterConfig {
public:
  // Server::Configuration::NamedHttpFilterConfigFactory
  std::string name() override {
    return Config::GFunctionFilterNames::get().GFUNCTION;
  }

  // Server::Configuration::EmptyHttpFilterConfig
  HttpFilterFactoryCb createFilter(const std::string &stat_prefix,
                                   FactoryContext &context) override {
    UNREFERENCED_PARAMETER(stat_prefix);

    return [&context](Http::FilterChainFactoryCallbacks &callbacks) -> void {
      auto filter = new MixedGFunctionFilter(
          context, Config::GFunctionFilterNames::get().GFUNCTION);
      callbacks.addStreamDecoderFilter(
          Http::StreamDecoderFilterSharedPtr{filter});
    };
  }
};

/**
 * Static registration for the Google Cloud Functions filter. @see
 * RegisterFactory.
 */
static Registry::RegisterFactory<GfunctionFilterFactory,
                                 NamedHttpFilterConfigFactory>
    register_;

} // namespace Configuration
} // namespace Server
} // namespace Envoy
