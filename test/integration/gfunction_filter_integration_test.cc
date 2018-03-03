#include "common/config/gfunction_well_known_names.h"
#include "common/config/metadata.h"

#include "test/integration/http_integration.h"
#include "test/integration/integration.h"
#include "test/integration/utility.h"

namespace Envoy {

const std::string DEFAULT_GFUNCTION_FILTER =
    R"EOF(
name: io.solo.gcloudfunc
)EOF";

class GfunctionFilterIntegrationTest
    : public HttpIntegrationTest,
      public testing::TestWithParam<Network::Address::IpVersion> {
public:
  GfunctionFilterIntegrationTest()
      : HttpIntegrationTest(Http::CodecClient::Type::HTTP1, GetParam()) {}

  /**
   * Initializer for an individual integration test.
   */
  void initialize() override {
    config_helper_.addFilter(DEFAULT_GFUNCTION_FILTER);

    config_helper_.addConfigModifier(
        [](envoy::config::bootstrap::v2::Bootstrap &bootstrap) {
          auto &google_cluster =
              (*bootstrap.mutable_static_resources()->mutable_clusters(0));

          auto *metadata = google_cluster.mutable_metadata();

          // create some value to mark this cluster as gfunction.
          Config::Metadata::mutableMetadataValue(
              *metadata, Config::GFunctionMetadataFilters::get().GFUNCTION,
              Config::MetadataGFunctionKeys::get().HOST)
              .set_string_value("dummy value");

          /////
          // TODO(yuval-k): use consts (filename mess)
          std::string functionalfilter = "io.solo.function_router";
          std::string functionsKey = "functions";

          // add the function spec in the cluster config.
          // TODO(yuval-k): extract this to help method (test utils?)
          ProtobufWkt::Struct *functionstruct =
              Config::Metadata::mutableMetadataValue(
                  *metadata, functionalfilter, functionsKey)
                  .mutable_struct_value();

          ProtobufWkt::Value &functionstructspecvalue =
              (*functionstruct->mutable_fields())["funcname"];
          ProtobufWkt::Struct *functionsspecstruct =
              functionstructspecvalue.mutable_struct_value();

          (*functionsspecstruct
                ->mutable_fields())[Config::MetadataGFunctionKeys::get().HOST]
              .set_string_value(
                  "us-central1-some-project-id.cloudfunctions.net");
          (*functionsspecstruct
                ->mutable_fields())[Config::MetadataGFunctionKeys::get().PATH]
              .set_string_value("/function-1");
        });

    config_helper_.addConfigModifier(
        [](envoy::config::filter::network::http_connection_manager::v2::
               HttpConnectionManager &hcm) {
          auto *metadata = hcm.mutable_route_config()
                               ->mutable_virtual_hosts(0)
                               ->mutable_routes(0)
                               ->mutable_metadata();
          std::string functionalfilter = "io.solo.function_router";
          std::string functionKey = "function";
          std::string clustername =
              hcm.route_config().virtual_hosts(0).routes(0).route().cluster();

          ProtobufWkt::Struct *clusterstruct =
              Config::Metadata::mutableMetadataValue(
                  *metadata, functionalfilter, clustername)
                  .mutable_struct_value();

          (*clusterstruct->mutable_fields())[functionKey].set_string_value(
              "funcname");
        });

    HttpIntegrationTest::initialize();

    codec_client_ =
        makeHttpConnection(makeClientConnection((lookupPort("http"))));
  }

  /**
   * Initializer for an individual integration test.
   */
  void SetUp() override { initialize(); }
};

INSTANTIATE_TEST_CASE_P(
    IpVersions, GfunctionFilterIntegrationTest,
    testing::ValuesIn(TestEnvironment::getIpVersionsForTest()));

TEST_P(GfunctionFilterIntegrationTest, Test1) {
  Http::TestHeaderMapImpl request_headers{
      {":method", "POST"}, {":authority", "www.solo.io"}, {":path", "/"}};

  sendRequestAndWaitForResponse(request_headers, 10, default_response_headers_,
                                10);

  std::string path = upstream_request_->headers().Path()->value().c_str();
  std::string host = upstream_request_->headers().Host()->value().c_str();

  EXPECT_EQ("us-central1-some-project-id.cloudfunctions.net", host);
  EXPECT_EQ("/function-1", path);
}

} // namespace Envoy
