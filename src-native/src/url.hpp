#ifndef GAME_MAPS_CORE_URL_HPP
#define GAME_MAPS_CORE_URL_HPP

#include <string>
#include "stdafx.h"

struct ParsedURL final {
  std::string scheme;
  std::string subdomain;
  std::string domain;
  std::string port;
  std::string path;
  std::string query;
  std::string fragment;
};

CORE_PRIVATE ParsedURL* parse_url(const std::string& url);

#endif // GAME_MAPS_CORE_URL_HPP