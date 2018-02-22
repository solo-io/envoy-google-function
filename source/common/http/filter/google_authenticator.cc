#include "common/http/filter/google_authenticator.h"

#include <algorithm>
#include <list>
#include <string>
#include <vector>

namespace Envoy {
namespace Http {

GoogleAuthenticator::GoogleAuthenticator(std::string &&access_key,
                                         std::string &&secret_key,
                                         std::string &&service)
    : access_key_(access_key), first_key_(secret_key), service_(service) {}

GoogleAuthenticator::~GoogleAuthenticator() {}

} // namespace Http
} // namespace Envoy
