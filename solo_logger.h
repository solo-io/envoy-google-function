#pragma once

#include <string>

#include "envoy/common/optional.h"

#include "envoy/http/async_client.h"
#include "envoy/upstream/cluster_manager.h"

#include "common/common/logger.h"


namespace solo {
namespace logger {

  struct RequestInfo {
    std::string request_id_;
    std::string function_name_;
    std::string region_;
    std::string project_;
    std::string provider_;
  };

  class CloudCollector : public Envoy::Logger::Loggable<Envoy::Logger::Id::filter> {
    public:
    CloudCollector(Envoy::Upstream::ClusterManager& cm);
    ~CloudCollector();

    void storeRequestInfo(RequestInfo& info, Envoy::Http::HeaderMap* headers);
    
    private:
    Envoy::Upstream::ClusterManager& cm_;
    Envoy::Event::TimerPtr delay_timer_;
    std::string cluster_name_;
    Envoy::Optional<std::chrono::milliseconds> timeout_;
    uint retry_count_;
    Envoy::Http::AsyncClient::Request* in_flight_request_;
  };
}
}