#ifndef GAME_MAPS_CORE_UTILS_HPP
#define GAME_MAPS_CORE_UTILS_HPP

#include <string>
#include <vector>
#include "stdafx.h"

CORE_PRIVATE int starts_with(const std::string& str, const std::string& prefix);

CORE_PRIVATE std::vector<std::string> split(const std::string& s, char delimiter);

CORE_PRIVATE uint64_t string_to_uint64_ss(const std::string& str);

CORE_PRIVATE void replace(std::string& str, const std::string& find_str, const std::string& replace_str);

CORE_PRIVATE void replace_range(std::string& str, const std::string& find_str, const std::string& replace_str, size_t start, size_t end);

#endif // GAME_MAPS_CORE_UTILS_HPP