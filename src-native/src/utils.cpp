#include <vector>
#include <regex>
#include "utils.hpp"

CORE_PRIVATE std::vector<std::string> splitHost(const std::string& host) {
  std::vector<std::string> result;
  size_t pos = host.find('.');
  if (pos != std::string::npos) {
    result.push_back(host.substr(0, pos)); // Subdomain
    result.push_back(host.substr(pos + 1)); // Domain
  } else {
    result.push_back(""); // No subdomain
    result.push_back(host); // Entire host is the domain
  }
  return result;
}

CORE_PRIVATE ParsedURL* parseURL(const std::string& url) {
  const std::regex urlRegex(R"(^(\w+):\/\/([^\/:]+)(?::(\d+))?(\/[^\?#]*)?(\?[^\#]*)?(#.*)?$)");
  std::smatch match;

  if (std::regex_match(url, match, urlRegex)) {
    auto result = new ParsedURL;
    result->scheme = match[1].str();
    std::vector<std::string> hostParts = splitHost(match[2].str());
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

CORE_PRIVATE int starts_with(const std::string& str, const std::string& prefix) {
  if (prefix.length() > str.length()) {
    return 0;
  }
  return str.compare(0, prefix.length(), prefix) == 0 ? 1 : 0;
}