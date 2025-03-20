#include "network-request.hpp"

CORE_EXTERN_C CORE_PUBLIC NetworkRequest* core_network_request_new(const char* url, const NetworkRequestMethod method) {
  const auto parsed_url = parse_url(url);
  if (!parsed_url) {
    return nullptr;
  }

  return new NetworkRequestImpl(url, method, parsed_url);
}

CORE_EXTERN_C CORE_PUBLIC void core_network_request_add_header(NetworkRequest* request, const char* value) {
  request->m_headers = curl_slist_append(request->m_headers, value);
}

CORE_EXTERN_C CORE_PUBLIC void core_network_request_add_content(NetworkRequest* request, const char* content) {
  if (request->m_content) {
    free(request->m_content);
  }

  // ReSharper disable once CppDeprecatedEntity
  request->m_content = strdup(content);
}

CORE_EXTERN_C CORE_PUBLIC void core_network_request_delete(const NetworkRequest* request) {
  delete request;
}

NetworkRequestImpl::NetworkRequestImpl(const char *url, const NetworkRequestMethod method, ParsedURL* parsed_url)
  : m_url(url)
  , m_method(method)
  , m_parsed_url(parsed_url)
{}

NetworkRequestImpl::~NetworkRequestImpl() {
  if (m_headers) {
    curl_slist_free_all(m_headers);
  }
  if (m_content) {
    free(m_content);
  }

  delete m_parsed_url;
}
