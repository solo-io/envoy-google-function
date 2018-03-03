#include "common/http/filter/function.h"

namespace Envoy {
namespace Http {

Optional<GFunction>
GFunction::createFunction(Optional<const std::string *> name,
                          Optional<const std::string *> host,
                          Optional<const std::string *> region,
                          Optional<const std::string *> project_) {
  if (!name.valid()) {
    return {};
  }
  if (!host.valid()) {
    return {};
  }
  if (!region.valid()) {
    return {};
  }
  if (!project_.valid()) {
    return {};
  }
  const GFunction f =
      GFunction(name.value(), host.value(), region.value(), project_.value());
  return Optional<GFunction>(f);
}

} // namespace Http
} // namespace Envoy
