#include "mapgenie-network.hpp"
#include "network-request.hpp"
#include "utils.hpp"

CORE_PRIVATE NetworkResponse* mapgenie_network_override(const NetworkRequest* request) {
  if (starts_with(request->m_url, "https://mapgenie.io/api/v1/user/locations/") == 0) {
  }
  if (starts_with(request->m_url, "https://mapgenie.io/api/v1/user/notes") == 0) {
  }
  if (starts_with(request->m_url, "https://mapgenie.io/api/local/map-data/") == 0) {
  }

  return nullptr;
}