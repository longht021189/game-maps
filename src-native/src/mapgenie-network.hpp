#ifndef GAME_MAPS_CORE_MAPGENIE_NETWORK_HPP
#define GAME_MAPS_CORE_MAPGENIE_NETWORK_HPP

#include "network.hpp"

CORE_PRIVATE NetworkResponse* core_mapgenie_network_override(const NetworkRequest* request);

CORE_PRIVATE void edit_map_html(NetworkResponse* response);

#endif // GAME_MAPS_CORE_MAPGENIE_NETWORK_HPP