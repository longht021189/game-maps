#include "network-fetch.hpp"
#include "network-request.hpp"
#include "network-response.hpp"
#include <mutex>
#include <curl/curl.h>
#include <sstream>

static volatile bool curl_initialized = false;

static std::mutex curl_initialization_mutex {};

CORE_PRIVATE void initCURL() {
  std::lock_guard guard(curl_initialization_mutex);
  if (!curl_initialized) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_initialized = true;
  }
}

CORE_PRIVATE size_t build_curl_write_function(
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

CORE_EXTERN_C CORE_PUBLIC NetworkResponse* core_network_fetch(const NetworkRequest* request) {
  initCURL();

  if (CURL *curl = curl_easy_init()) {
    std::stringstream data{};
    std::stringstream headers_out{};
    char error_message[CURL_ERROR_SIZE];

    switch (request->m_method) {
      case NETWORK_REQUEST_METHOD_GET:
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
      break;

      case NETWORK_REQUEST_METHOD_DELETE:
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
      break;

      case NETWORK_REQUEST_METHOD_POST:
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request->m_content);
        break;

      case NETWORK_REQUEST_METHOD_PUT:
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request->m_content);
        break;

      default:
        break;
    }

    curl_easy_setopt(curl, CURLOPT_URL, request->m_url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "zstd, gzip, deflate");
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_DNS_CACHE_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, request->m_headers);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_message);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, build_curl_write_function);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, build_curl_write_function);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headers_out);

    const auto curl_code = curl_easy_perform(curl);
    NetworkResponse* response = nullptr;

    if (curl_code == CURLE_OK) {
      response = static_cast<NetworkResponse*>(calloc(1, sizeof(NetworkResponse)));
      response->status = NETWORK_RESPONSE_STATUS_OK;

      // ReSharper disable once CppDeprecatedEntity
      response->content = strdup(data.str().c_str());

      // ReSharper disable once CppDeprecatedEntity
      response->headers = strdup(headers_out.str().c_str());
    } else {
      response = &NetworkResponseErrors::get()->error_not_found;

      // TODO Report Error
      // std::cerr << "curl_easy_perform() failed: "
      //           << curl_easy_strerror(curl_code) << std::endl;
    }

    curl_easy_cleanup(curl);

    return response;
  }

  return &NetworkResponseErrors::get()->error_not_found;
}