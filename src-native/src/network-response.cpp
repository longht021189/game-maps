#include <cstdlib>
#include <vector>
#include "network-response.hpp"

NetworkResponseErrors errors {};

CORE_EXTERN_C CORE_PUBLIC void core_network_response_delete(NetworkResponse* response) {
  if (std::ranges::find(errors.all, response) == errors.all.end()) {
    if (response->content) {
      free(response->content);
    }
    if (response->headers) {
      free(response->headers);
    }
    free(response);
  }
}

NetworkResponseErrors::NetworkResponseErrors() {
  all.push_back(&error_not_found);
  all.push_back(&error_method_not_allowed);
  all.push_back(&error_no_content);
}

NetworkResponseErrors* NetworkResponseErrors::get() {
  return &errors;
}