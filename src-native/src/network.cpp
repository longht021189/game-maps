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

CORE_EXTERN_C CORE_PUBLIC NetworkFilter* core_network_filters() {
  NetworkFilter* result = nullptr;

  for (
    const auto& ads_domains = core_network_ads_domains();
    const auto& domain : ads_domains
  ) {
    auto filter1 = "https://" + domain + "/*";
    auto filter2 = "https://*." + domain + "/*";

    const auto head = new NetworkFilter {
      .next = new NetworkFilter {
        .next = result,
        // ReSharper disable once CppDeprecatedEntity
        .value = strdup(filter2.c_str())
      },
      // ReSharper disable once CppDeprecatedEntity
      .value = strdup(filter1.c_str())
    };

    result = head;
  }

  for (
    const auto& mapgenie_domains = core_mapgenie_network_filters();
    const auto& domain : mapgenie_domains
  ) {
    const auto head = new NetworkFilter {
      .next = result,

      // ReSharper disable once CppDeprecatedEntity
      .value = strdup(domain.c_str())
    };

    result = head;
  }

  return result;
}

CORE_EXTERN_C CORE_PUBLIC void core_network_filters_delete(NetworkFilter* value) {
  while (value != nullptr) {
    free(value->value);
    const auto current = value;
    value = value->next;
    free(current);
  }
}