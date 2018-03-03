#pragma once

#include <map>
#include <string>
#include <tuple>

#include "envoy/common/optional.h"

#include "common/protobuf/utility.h"

namespace Envoy {
namespace Http {

struct GFunction {

  // TODO(yuval-k): Remove this when we have a optional that can support types
  // without a default ctor.
  Function()
      : name_(nullptr), host_(nullptr), region_(nullptr), project_(nullptr),
        func_name_(nullptr) {}

  Function(const std::string *name, const std::string *host,
           const std::string *region, const std::string *project)
      : name_(name), qualifier_(qualifier), async_(async), host_(host),
        region_(region), project_(project), func_name_(func_name) {}

  static Optional<Function>
  createFunction(Optional<const std::string *> name,
                 Optional<const std::string *> host,
                 Optional<const std::string *> region,
                 Optional<const std::string *> project);

  const std::string *name_{nullptr};
  const std::string *host_{nullptr};
  const std::string *region_{nullptr};
  const std::string *project_{nullptr};
};

} // namespace Http
} // namespace Envoy
