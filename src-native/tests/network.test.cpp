#include <string>
#include <catch2/catch_test_macros.hpp>
#include <ylt/easylog.hpp>
#include "network.h"
#include "utils.hpp"

TEST_CASE("Check URL Format", "[network][parse][url]") {
  const std::string url = "https://sub.example.com:8080/path/to/resource?query=123#fragment";
  auto parsed = parseURL(url);
  REQUIRE(parsed != nullptr);
  ELOG_INFO << "Scheme: " << parsed->scheme;
  ELOG_INFO << "Subdomain: " << parsed->subdomain;
  ELOG_INFO << "Domain: " << parsed->domain;
  ELOG_INFO << "Port: " << parsed->port;
  ELOG_INFO << "Path: " << parsed->path;
  ELOG_INFO << "Query: " << parsed->query;
  ELOG_INFO << "Fragment: " << parsed->fragment;
  delete parsed;
}