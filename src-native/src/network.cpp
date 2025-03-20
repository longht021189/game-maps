#include "network.hpp"
#include "network-ads.hpp"
#include "network-response.hpp"
#include "mapgenie-network.hpp"

CORE_EXTERN_C CORE_PUBLIC NetworkResponse* core_network_override(NetworkRequest* request) {
  if (core_network_is_ads(request)) {
    return &NetworkResponseErrors::get()->error_not_found;
  }
  if (const auto response = core_mapgenie_network_override(request)) {
    return response;
  }
  return nullptr;
}