#include "common/http/filter/solo_logger.h"

#include "envoy/http/header_map.h"

#include "common/common/empty_string.h"
#include "common/common/hex.h"
#include "common/common/utility.h"
#include "common/http/message_impl.h"

#include "server/config/network/http_connection_manager.h"

#define COPY_INLINE_HEADER(name, from, to)                                     \
  if (from->name() != nullptr)                                                 \
    to->insert##name().value(std::string(from->name()->value().c_str()));

namespace Envoy {
namespace Http {

CloudCollector::CloudCollector(Envoy::Upstream::ClusterManager &cm,
                               CallbackerSharedPtr cb)
    : cm_(cm), callbacks_(cb), delay_timer_(nullptr), cluster_name_("sololog"),
      timeout_(std::chrono::milliseconds(1000)), retry_count_(0),
      in_flight_request_(nullptr) {}

CloudCollector::~CloudCollector() {}

void CloudCollector::storeRequestInfo(CloudCollector::RequestInfo &info,
                                      Envoy::Http::HeaderMap *tracing_headers) {

  Envoy::Http::MessagePtr request(new Envoy::Http::RequestMessageImpl());
  request->headers().insertContentType().value(std::string("application/json"));
  request->headers().insertPath().value(std::string("/api/v1/store"));
  request->headers().insertHost().value(std::string("sololog"));
  request->headers().insertMethod().value(std::string("POST"));

  if (tracing_headers != nullptr) {
    Envoy::Http::HeaderMap *h = &(request->headers());
    COPY_INLINE_HEADER(RequestId, tracing_headers, h);
    COPY_INLINE_HEADER(XB3TraceId, tracing_headers, h);
    COPY_INLINE_HEADER(XB3SpanId, tracing_headers, h);
    COPY_INLINE_HEADER(XB3ParentSpanId, tracing_headers, h);
    COPY_INLINE_HEADER(XB3Sampled, tracing_headers, h);
    COPY_INLINE_HEADER(OtSpanContext, tracing_headers, h);
  }

  std::string body = "{\"requestId\":\"" + info.request_id_ +
                     "\",\"function\":\"" + info.function_name_ +
                     "\",\"region\":\"" + info.region_ + "\",\"project\":\"" +
                     info.project_ + "\",\"provider\":\"" + info.provider_ +
                     "\"}";

  request->body().reset(new Envoy::Buffer::OwnedImpl(body));

  in_flight_request_ =
      cm_.httpAsyncClientForCluster(cluster_name_)
          .send(std::move(request), *(callbacks_.get()), timeout_);
  ENVOY_LOG(debug,
            "SOLO_LOGGER: Async request to store tracing info was sent to "
            "cluster: \"{}\"",
            cluster_name_);
}
} // namespace Http
} // namespace Envoy
