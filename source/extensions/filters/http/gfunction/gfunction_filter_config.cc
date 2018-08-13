#include <string>

#include "envoy/registry/registry.h"

#include "common/config/gfunction_well_known_names.h"
#include "common/http/functional_stream_decoder_base.h"

#include "extensions/filters/http/common/empty_http_filter_config.h"
#include "extensions/filters/http/gfunction/gfunction_filter.h"

namespace Envoy {
namespace Server {
namespace Configuration {

using Extensions::HttpFilters::Common::EmptyHttpFilterConfig;

typedef Http::FunctionalFilterMixin<Http::GfunctionFilter> MixedGFunctionFilter;

/**
 * Config registration for the Google Cloud Functions filter.
 */
class GfunctionFilterFactory : public EmptyHttpFilterConfig {
public:
  GfunctionFilterFactory()
      : EmptyHttpFilterConfig(Config::GFunctionFilterNames::get().GFUNCTION) {}

  Http::FilterFactoryCb createFilter(const std::string &stat_prefix,
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
