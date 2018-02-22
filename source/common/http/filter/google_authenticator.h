#pragma once

#include <string>


namespace Solo {
namespace Gfunction {

class GoogleAuthenticator {
public:
  GoogleAuthenticator(std::string&& access_key, std::string&& secret_key, std::string&& service);    
  ~GoogleAuthenticator();

private: 

  std::string access_key_;
  std::string first_key_;
  std::string service_;

};

} // Gfunction
} // Solo