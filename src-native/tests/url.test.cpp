#include "url.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Test basic URL parsing", "[game][maps][core][url][1]") {
  ParsedURL* result = parse_url("https://www.example.com/path");
  REQUIRE_FALSE(nullptr == result);
  REQUIRE("https" == result->scheme);
  REQUIRE("www" == result->subdomain);
  REQUIRE("example.com" == result->domain);
  REQUIRE(result->port.empty());
  REQUIRE("/path" == result->path);
  REQUIRE(result->query.empty());
  REQUIRE(result->fragment.empty());
  delete result;
}

TEST_CASE("Test URL with all components", "[game][maps][core][url][2]") {
  ParsedURL* result = parse_url("http://sub.domain.com:8080/path/to/file?key=value#section1");
  REQUIRE_FALSE(nullptr == result);
  REQUIRE("http" == result->scheme);
  REQUIRE("sub" == result->subdomain);
  REQUIRE("domain.com" == result->domain);
  REQUIRE("8080" == result->port);
  REQUIRE("/path/to/file" == result->path);
  REQUIRE("?key=value" == result->query);
  REQUIRE("#section1" == result->fragment);
  delete result;
}

TEST_CASE("Test minimal URL", "[game][maps][core][url][3.1]") {
  ParsedURL* result = parse_url("http://example.com");
  REQUIRE_FALSE(nullptr == result);
  REQUIRE("http" == result->scheme);
  REQUIRE(result->subdomain.empty());
  REQUIRE("example.com" == result->domain);
  REQUIRE(result->port.empty());
  REQUIRE(result->path.empty());
  REQUIRE(result->query.empty());
  REQUIRE(result->fragment.empty());
  delete result;
}

TEST_CASE("Test minimal URL with www", "[game][maps][core][url][3.2]") {
  ParsedURL* result = parse_url("https://www.example.com");
  REQUIRE_FALSE(nullptr == result);
  REQUIRE("https" == result->scheme);
  REQUIRE(result->subdomain.empty());
  REQUIRE("example.com" == result->domain);
  REQUIRE(result->port.empty());
  REQUIRE(result->path.empty());
  REQUIRE(result->query.empty());
  REQUIRE(result->fragment.empty());
  delete result;
}

TEST_CASE("Test URL with multiple query parameters", "[game][maps][core][url][4]") {
  ParsedURL* result = parse_url("https://test.com/path?a=1&b=2");
  REQUIRE_FALSE(nullptr == result);
  REQUIRE("https" == result->scheme);
  REQUIRE(result->subdomain.empty());
  REQUIRE("test.com" == result->domain);
  REQUIRE(result->port.empty());
  REQUIRE("/path" == result->path);
  REQUIRE("?a=1&b=2" == result->query);
  REQUIRE(result->fragment.empty());;
  delete result;
}

TEST_CASE("Test empty URL", "[game][maps][core][url][5]") {
  ParsedURL* result = parse_url("");
  REQUIRE(nullptr == result);
}

TEST_CASE("Test malformed URL", "[game][maps][core][url][6]") {
  ParsedURL* result = parse_url("not_a_valid_url");
  REQUIRE(nullptr == result);
}
