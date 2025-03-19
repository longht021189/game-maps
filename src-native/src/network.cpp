#include <curl/curl.h>
#include <sstream>
#include <vector>
#include <regex>
#include <mutex>
#include "network.h"
#include "ads.hpp"
#include "mapgenie-network.hpp"
#include "network-request.hpp"

static volatile bool curl_initialized = false;

static std::mutex curl_initialization_mutex {};

CORE_PRIVATE void initCURL() {
  std::lock_guard guard(curl_initialization_mutex);
  if (!curl_initialized) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_initialized = true;
  }
}

CORE_EXTERN_C CORE_PUBLIC NetworkRequest* core_network_request_new(const char* url, const NetworkRequestMethod method) {
  auto parsedURL = parseURL(url);
  if (!parsedURL) {
    return nullptr;
  }

  return new NetworkRequestImpl(url, parsedURL, method);
}

CORE_EXTERN_C CORE_PUBLIC void core_network_request_add_header(NetworkRequest* request, const char* header) {
  request->m_headers = curl_slist_append(request->m_headers, header);
}

CORE_EXTERN_C CORE_PUBLIC void core_network_request_set_content(NetworkRequest* request, const char* content, const NetworkRequestContentType content_type) {
  request->m_content = strdup(content);
  request->m_content_type = content_type;
}

CORE_EXTERN_C CORE_PUBLIC void core_network_request_delete(const NetworkRequest* request) {
  delete request;
}

CORE_EXTERN_C CORE_PUBLIC NetworkResponse* core_network_override(const NetworkRequest* request) {
  if (is_ads(request->m_parsed_url)) {
    const auto response = static_cast<NetworkResponse*>(calloc(1, sizeof(NetworkResponse)));
    response->status = NETWORK_RESPONSE_STATUS_NOT_FOUND;
    return response;
  }

  initCURL();

  return mapgenie_network_override(request);
}

CORE_EXTERN_C CORE_PUBLIC void core_network_response_delete(NetworkResponse* response) {
  free(response);
}

/*


extern "C" EXPORT void add_header(Request *request, const char *value) {
  request->headers = curl_slist_append(request->headers, value);
}

extern "C" EXPORT void free_headers(Request *request) {
  if (request->headers != nullptr) {
    curl_slist_free_all(request->headers);
    request->headers = nullptr;
  }
}

size_t build_curl_write_function(
  const void *input,
  const size_t size,
  const size_t block,
  std::stringstream *response
) {
  const size_t count = size * block;
  const std::string value(static_cast<const char *>(input), count);
  (*response) << value;
  return count;
}

extern "C" EXPORT Response *fetch(Request *request) {
  if (!curl_initialized) {

    curl_initialized = true;
  }

  CURL *curl = curl_easy_init();

  if (curl) {
    std::stringstream data{};
    char error_message[CURL_ERROR_SIZE];

    switch (request->method) {
      case REQUEST_METHOD_GET:
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
        break;

      case REQUEST_METHOD_DELETE:
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        break;

      case REQUEST_METHOD_POST:
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request->content);

      // input.headers = build_curl_set_body(
      //     input.headers,
      //     curl,
      //     input.request.body,
      //     input.request.body_type
      // );
        break;

      case REQUEST_METHOD_PUT:
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request->content);

      // input.headers = build_curl_set_body(
      //     input.headers,
      //     curl,
      //     input.request.body,
      //     input.request.body_type
      // );
        break;

      default:
        break;
    }

    curl_easy_setopt(curl, CURLOPT_URL, request->url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "zstd, gzip, deflate");
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_DNS_CACHE_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, request->headers);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_message);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, build_curl_write_function);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
  }

  return nullptr;
}

extern "C" EXPORT void free_response(Response *response) {
  if (response != nullptr) {
    delete response;
  }
}
*/