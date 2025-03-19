#ifndef GAME_MAPS_CORE_UTILS_H
#define GAME_MAPS_CORE_UTILS_H

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

CORE_PRIVATE ParsedURL* parseURL(const std::string& url);

CORE_PRIVATE int starts_with(const std::string& str, const std::string& prefix);

#endif // GAME_MAPS_CORE_UTILS_H