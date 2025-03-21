#include "network-fetch.hpp"
#include "network-request.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Test fetch google page", "[game][maps][core][fetch][1]") {
  const auto request = core_network_request_new("https://www.google.com", NETWORK_REQUEST_METHOD_GET);
  const auto response = core_network_fetch(request);
  REQUIRE(NETWORK_RESPONSE_STATUS_OK == response->status);
  core_network_request_delete(request);
  core_network_response_delete(response);
}

TEST_CASE("Test fetch invalid page", "[game][maps][core][fetch][2]") {
  const auto request = core_network_request_new("https://asreredfsdf.aasdasd", NETWORK_REQUEST_METHOD_GET);
  const auto response = core_network_fetch(request);
  REQUIRE(NETWORK_RESPONSE_STATUS_NOT_FOUND == response->status);
  core_network_request_delete(request);
  core_network_response_delete(response);
}