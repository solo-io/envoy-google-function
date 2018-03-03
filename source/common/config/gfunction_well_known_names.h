#pragma once

#include <string>

#include "common/singleton/const_singleton.h"

namespace Envoy {
namespace Config {

// TODO(talnordan): TODO: Merge with
// envoy/source/common/config/well_known_names.h.

/**
 * Well-known http filter names.
 */
class GFunctionFilterNameValues {
public:
  // Google Cloud Functions filter
  const std::string GFUNCTION = "io.solo.gcloudfunc";
};

typedef ConstSingleton<GFunctionFilterNameValues> GFunctionFilterNames;

/**
 * Well-known metadata filter namespaces.
 */
class GFunctionMetadataFilterValues {
public:
  // Filter namespace for Google Cloud Functions Filter.
  const std::string GFUNCTION = "io.solo.gcloudfunc";
};

typedef ConstSingleton<GFunctionMetadataFilterValues> GFunctionMetadataFilters;

/**
 * Keys for GFunctionMetadataFilterValues::GFUNCTION metadata.
 */
class MetadataGFunctionKeyValues {
public:
  const std::string HOST = "host";
  const std::string PATH = "path";
};

typedef ConstSingleton<MetadataGFunctionKeyValues> MetadataGFunctionKeys;

} // namespace Config
} // namespace Envoy
