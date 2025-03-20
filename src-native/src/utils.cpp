#include "utils.hpp"
#include <sstream>

CORE_PRIVATE int starts_with(const std::string& str, const std::string& prefix) {
  if (prefix.length() > str.length()) {
    return 0;
  }
  return str.compare(0, prefix.length(), prefix) == 0 ? 1 : 0;
}

CORE_PRIVATE std::vector<std::string> split(const std::string& s, const char delimiter) {
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(s);
  while (std::getline(tokenStream, token, delimiter)) {
    if (!token.empty()) {
      tokens.push_back(token);
    }
  }
  return tokens;
}

CORE_PRIVATE uint64_t string_to_uint64_ss(const std::string& str) {
  std::stringstream ss(str);
  uint64_t result;

  ss >> result;

  // Check if conversion failed or if there are leftover characters
  if (ss.fail() || !ss.eof()) {
    throw std::invalid_argument("Invalid uint64_t format: " + str);
  }

  return result;
}

CORE_PRIVATE void replace(
  std::string& str,
  const std::string& find_str,
  const std::string& replace_str
) {
  if (str.empty()) {
    return;
  }

  const auto len1 = find_str.length();
  if (len1 > str.length() || len1 <= 0) {
    return;
  }

  const auto len2 = replace_str.length();

  size_t pos = 0;
  while ((pos = str.find(find_str, pos)) != std::string::npos) {
    str.replace(pos, len1, replace_str);
    pos += len2;
  }
}

CORE_PRIVATE void replace_range(
  std::string& str,
  const std::string& find_str,
  const std::string& replace_str,
  const size_t start,
  size_t end
) {
  if (str.empty() || start >= end) {
    return;
  }

  const auto len1 = find_str.length();
  if (len1 > str.length() || len1 <= 0) {
    return;
  }

  const auto len2 = replace_str.length();

  // Adjust end if it exceeds string length
  end = std::min(end, str.length());

  size_t pos = start;
  while ((pos = str.find(find_str, pos)) != std::string::npos && pos < end) {
    str.replace(pos, len1, replace_str);
    pos += len2;

    // Adjust end if replacement changes string length
    if (pos >= end) break;
    end += len2 - len1;
  }
}