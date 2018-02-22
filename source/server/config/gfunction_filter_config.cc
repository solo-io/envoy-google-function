#include <string>

#include "common/http/filter/gfunction_filter.h"

#include "envoy/registry/registry.h"

#include "common/http/filter/solo_logger.h"

namespace Envoy {
namespace Server {
namespace Configuration {
  
const std::string gFUNCTION_HTTP_FILTER_SCHEMA(R"EOF(
  {
    "$schema": "http://json-schema.org/schema#",
    "type" : "object",
    "properties" : {
      "access_key": {
        "type" : "string"
      },
      "secret_key": {
        "type" : "string"
      },
      "functions": {
        "type" : "object",
        "additionalProperties" : {
          "type" : "object",
          "properties": {
            "func_name" : {"type":"string"},
            "hostname" : {"type":"string"},
            "region" : {"type":"string"},
            "project" : {"type":"string"}
          }
        }
      }
    },
    "required": ["access_key", "secret_key", "functions"],
    "additionalProperties" : false
  }
  )EOF");

class GfunctionFilterFactory : public Envoy::Server::Configuration::NamedHttpFilterConfigFactory {
public:
  Envoy::Server::Configuration::HttpFilterFactoryCb createFilterFactory(const Envoy::Json::Object& json_config, const std::string&,
    Envoy::Server::Configuration::FactoryContext& context) override {
    json_config.validateSchema(gFUNCTION_HTTP_FILTER_SCHEMA);
                     
  std::string access_key = json_config.getString("access_key", "");
  std::string secret_key = json_config.getString("secret_key", "");
  const Envoy::Json::ObjectSharedPtr functions_obj = json_config.getObject("functions", false);

  //Solo::Logger::Callbacker cb;
  Http::CallbackerSharedPtr cb = std::make_shared<Http::Callbacker>();

  Http::ClusterFunctionMap functions;

  functions_obj->iterate([&functions](const std::string& key, const Envoy::Json::Object& value){
    const std::string cluster_name = key;
    const std::string func_name = value.getString("func_name", "");
    const std::string hostname = value.getString("hostname", "");
    const std::string region = value.getString("region", "");
    const std::string project = value.getString("project", "");
    functions[cluster_name] = Http::Function {
      .func_name_ = func_name,
      .hostname_ = hostname,
      .region_  = region,
      .project_ = project,
    };
    return true;
  });

    return [&context, cb, access_key, secret_key, functions](Http::FilterChainFactoryCallbacks& callbacks) -> void {
      auto filter = new Http::GfunctionFilter(
        context.clusterManager(),
        std::move(cb), 
        std::move(access_key), 
        std::move(secret_key), 
        std::move(functions));
      callbacks.addStreamFilter(
        Envoy::Http::StreamFilterSharedPtr{filter});
    };
  }
  std::string name() override { return "Gfunction"; }
};

/**
 * Static registration for this sample filter. @see RegisterFactory.
 */
static Envoy::Registry::RegisterFactory<GfunctionFilterFactory, Envoy::Server::Configuration::NamedHttpFilterConfigFactory>
    register_;

} // Configuration
} // Gfunction
} // Solo
