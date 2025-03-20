#include <regex>
#include <vector>
#include <sstream>
#include "url.hpp"
#include "utils.hpp"

CORE_PRIVATE std::vector<std::string> split_host(const std::string& host) {
  std::vector<std::string> result {};
  result.emplace_back("");
  result.push_back(host);

  // Split the host into parts by '.'
  const std::vector<std::string> parts = split(host, '.');

  // Minimum valid case is "domain.tld" (2 parts)
  // if (parts.size() < 2) {
  //   return result;  // Return empty parts for invalid input
  // }

  // Common TLDs (this is a simplified list)
  // std::vector<std::string> common_tlds = { "com", "org", "net", "co", "io" };

  // if (parts.size() == 2) {
  //   // Case: "example.com"
  //   result.main_domain = parts[0] + "." + parts[1];
  //   result.subdomain = "";
  // } else

  if (parts.size() >= 3) {
    // Case: "www.example.com" or "sub.example.com"
    // Get the last two parts as the main domain for simplicity
    // Note: This is a basic implementation and might need adjustment for complex TLDs
    result[1] = parts[parts.size() - 2] + "." + parts[parts.size() - 1];

    // Join all parts before the main domain as subdomain
    std::string subdomain;
    for (size_t i = 0; i < parts.size() - 2; ++i) {
      if (i > 0) subdomain += ".";
      subdomain += parts[i];
    }
    result[0] = subdomain;
  }

  return result;
}

CORE_PRIVATE ParsedURL* parse_url(const std::string& url) {
  const std::regex urlRegex(R"(^(\w+):\/\/([^\/:]+)(?::(\d+))?(\/[^\?#]*)?(\?[^\#]*)?(#.*)?$)");
  std::smatch match;

  if (std::regex_match(url, match, urlRegex)) {
    auto result = new ParsedURL;
    result->scheme = match[1].str();
    std::vector<std::string> hostParts = split_host(match[2].str());
    result->subdomain = hostParts[0];
    result->domain = hostParts[1];
    result->port = match[3].str();
    result->path = match[4].str();
    result->query = match[5].str();
    result->fragment = match[6].str();
    return result;
  }

  return nullptr;
}