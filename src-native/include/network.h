#ifndef GAME_MAPS_CORE_NETWORK_H
#define GAME_MAPS_CORE_NETWORK_H

#include "stdafx.h"
#include "network-request.h"
#include "network-response.h"

CORE_EXTERN_C_BEGIN

CORE_PUBLIC NetworkResponse* core_network_override(NetworkRequest* request);

CORE_PUBLIC NetworkResponse* core_network_fetch(const NetworkRequest* request);

CORE_EXTERN_C_END

#endif // GAME_MAPS_CORE_NETWORK_H