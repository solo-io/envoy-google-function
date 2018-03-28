#include <string>

#include "envoy/registry/registry.h"

#include "common/config/gfunction_well_known_names.h"
#include "common/http/filter/gfunction_filter.h"
#include "common/http/functional_stream_decoder_base.h"

#include "google_func_filter.pb.validate.h"

namespace Envoy {
namespace Server {
namespace Configuration {

typedef Http::FunctionalFilterMixin<Http::GfunctionFilter> MixedGFunctionFilter;

class GfunctionFilterFactory : public NamedHttpFilterConfigFactory {
public:
  HttpFilterFactoryCb createFilterFactory(const Json::Object &,
                                          const std::string &,
                                          FactoryContext &) override {
    NOT_IMPLEMENTED;
  }

  ProtobufTypes::MessagePtr createEmptyConfigProto() override {
    return ProtobufTypes::MessagePtr{
        new envoy::api::v2::filter::http::GoogleFunc()};
  }

  HttpFilterFactoryCb
  createFilterFactoryFromProto(const Protobuf::Message &config,
                               const std::string &stat_prefix,
                               FactoryContext &context) override {
    UNREFERENCED_PARAMETER(config);
    UNREFERENCED_PARAMETER(stat_prefix);

    // const auto& gconfig = dynamic_cast<const
    // envoy::api::v2::filter::http::GoogleFunc &>(config);

    // Solo::Logger::Callbacker cb;

    return [&context](Http::FilterChainFactoryCallbacks &callbacks) -> void {
      auto filter = new MixedGFunctionFilter(
          context, Config::GFunctionFilterNames::get().GFUNCTION);
      callbacks.addStreamDecoderFilter(
          Http::StreamDecoderFilterSharedPtr{filter});
    };
  }
  std::string name() override {
    return Config::GFunctionFilterNames::get().GFUNCTION;
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
