#pragma once

#include <string>

#include "envoy/common/optional.h"
#include "envoy/http/async_client.h"
#include "envoy/upstream/cluster_manager.h"

#include "common/common/logger.h"

namespace Envoy {
namespace Http {

// Class to provide required callbacks to the AsyncClient send method
class Callbacker : public AsyncClient::Callbacks {
public:
  void onSuccess(MessagePtr &&) override {}
  void onFailure(AsyncClient::FailureReason) override {}
};
typedef std::shared_ptr<Callbacker> CallbackerSharedPtr;

class CloudCollector : public Logger::Loggable<Logger::Id::filter> {
public:
  struct RequestInfo {
    std::string request_id_;
    std::string function_name_;
    std::string region_;
    std::string project_;
    std::string provider_;
  };

  CloudCollector(Upstream::ClusterManager &cm, CallbackerSharedPtr cb);
  ~CloudCollector();

  void storeRequestInfo(CloudCollector::RequestInfo &info, HeaderMap *headers);

private:
  Upstream::ClusterManager &cm_;
  CallbackerSharedPtr callbacks_;
  Event::TimerPtr delay_timer_;
  std::string cluster_name_;
  Optional<std::chrono::milliseconds> timeout_;
  uint retry_count_;
  AsyncClient::Request *in_flight_request_;
};
} // namespace Http
} // namespace Envoy
