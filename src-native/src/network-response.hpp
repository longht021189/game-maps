#ifndef GAME_MAPS_CORE_NETWORK_RESPONSE_HPP
#define GAME_MAPS_CORE_NETWORK_RESPONSE_HPP

#include <vector>
#include "network-response.h"

struct NetworkResponseErrors final {
  std::vector<NetworkResponse*> all {};
  NetworkResponse error_not_found {.status = NETWORK_RESPONSE_STATUS_NOT_FOUND, .content = nullptr, .headers = nullptr};
  NetworkResponse error_method_not_allowed {.status = NETWORK_RESPONSE_STATUS_METHOD_NOT_ALLOWED, .content = nullptr, .headers = nullptr};
  NetworkResponse error_no_content {.status = NETWORK_RESPONSE_STATUS_NO_CONTENT, .content = nullptr, .headers = nullptr};

  NetworkResponseErrors();

  ~NetworkResponseErrors() = default;

  static NetworkResponseErrors* get();
};

#endif // GAME_MAPS_CORE_NETWORK_RESPONSE_HPP