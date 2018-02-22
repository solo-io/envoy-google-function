#include <string>
#include <algorithm>
#include <vector>
#include <list>

#include "common/http/filter/google_authenticator.h"


namespace Envoy {
namespace Http {


GoogleAuthenticator::GoogleAuthenticator(std::string&& access_key, std::string&& secret_key, std::string&& service) : 
  access_key_(access_key), first_key_(secret_key), service_(service) {
   
}

GoogleAuthenticator::~GoogleAuthenticator(){

}

} // Gfunction
} // Solo
