#ifndef GAME_MAPS_CORE_NETWORK_REQUEST_HPP
#define GAME_MAPS_CORE_NETWORK_REQUEST_HPP

#include <string>
#include <curl/curl.h>
#include "network-request.h"
#include "url.hpp"

struct NetworkRequestImpl final {
  std::string m_url;
  NetworkRequestMethod m_method;
  curl_slist *m_headers = nullptr;
  char *m_content = nullptr;
  ParsedURL* m_parsed_url;

  NetworkRequestImpl(const char* url, NetworkRequestMethod method, ParsedURL* parsed_url);

  ~NetworkRequestImpl();
};

#endif // GAME_MAPS_CORE_NETWORK_REQUEST_HPP