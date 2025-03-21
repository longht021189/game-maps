#ifndef GAME_MAPS_CORE_NETWORK_ADS_HPP
#define GAME_MAPS_CORE_NETWORK_ADS_HPP

#include <vector>
#include <string>
#include "network-request.hpp"

CORE_PRIVATE bool core_network_is_ads(const NetworkRequest* request);

CORE_PRIVATE const std::vector<std::string>& core_network_ads_domains();

#endif // GAME_MAPS_CORE_NETWORK_ADS_HPP