#include <string>

#include "envoy/registry/registry.h"

#include "common/config/gfunction_well_known_names.h"
#include "common/http/filter/gfunction_filter.h"
#include "common/http/filter/solo_logger.h"
#include "common/http/functional_stream_decoder_base.h"

#include "google_func_filter.pb.validate.h"

namespace Envoy {
namespace Server {
namespace Configuration {

typedef Http::FunctionalFilterMixin<Http::GfunctionFilter> MixedGFunctionFilter;

class GfunctionFilterFactory
    : public Envoy::Server::Configuration::NamedHttpFilterConfigFactory {
public:
  Envoy::Server::Configuration::HttpFilterFactoryCb
  createFilterFactory(const Envoy::Json::Object &, const std::string &,
                      Envoy::Server::Configuration::FactoryContext &) override {
    NOT_IMPLEMENTED;
  }

  ProtobufTypes::MessagePtr createEmptyConfigProto() {
    return ProtobufTypes::MessagePtr{
        new envoy::api::v2::filter::http::GoogleFunc()};
  }

  HttpFilterFactoryCb
  createFilterFactoryFromProto(const Protobuf::Message &config,
                               const std::string &stat_prefix,
                               FactoryContext &context) {
    UNREFERENCED_PARAMETER(config);
    UNREFERENCED_PARAMETER(stat_prefix);

    // const auto& gconfig = dynamic_cast<const
    // envoy::api::v2::filter::http::GoogleFunc &>(config);

    // Solo::Logger::Callbacker cb;
    Http::CallbackerSharedPtr cb = std::make_shared<Http::Callbacker>();

    return
        [&context, cb](Http::FilterChainFactoryCallbacks &callbacks) -> void {
          auto filter = new MixedGFunctionFilter(
              context, Config::GFunctionFilterNames::get().GFUNCTION,
              context.clusterManager(), cb);
          callbacks.addStreamFilter(Envoy::Http::StreamFilterSharedPtr{filter});
        };
  }
  std::string name() override {
    return Config::GFunctionFilterNames::get().GFUNCTION;
  }
};

/**
 * Static registration for this sample filter. @see RegisterFactory.
 */
static Envoy::Registry::RegisterFactory<
    GfunctionFilterFactory,
    Envoy::Server::Configuration::NamedHttpFilterConfigFactory>
    register_;

} // namespace Configuration
} // namespace Server
} // namespace Envoy
