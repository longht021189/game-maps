#ifndef GAME_MAPS_CORE_NETWORK_H
#define GAME_MAPS_CORE_NETWORK_H

#include "stdafx.h"
#include "network-request.h"
#include "network-response.h"

struct NetworkFilter {
  REMOVE_IN_CPP(struct) NetworkFilter* next;
  char* value;
};

CORE_EXTERN_C_BEGIN

CORE_PUBLIC NetworkResponse* core_network_override(NetworkRequest* request);

CORE_PUBLIC NetworkResponse* core_network_fetch(const NetworkRequest* request);

CORE_PUBLIC REMOVE_IN_CPP(struct) NetworkFilter* core_network_filters();

CORE_PUBLIC void core_network_filters_delete(REMOVE_IN_CPP(struct) NetworkFilter* value);

CORE_EXTERN_C_END

#endif // GAME_MAPS_CORE_NETWORK_H