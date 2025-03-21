#ifndef GAME_MAPS_CORE_NETWORK_REQUEST_H
#define GAME_MAPS_CORE_NETWORK_REQUEST_H

#include "stdafx.h"

struct NetworkRequestImpl;

typedef REMOVE_IN_CPP(struct) NetworkRequestImpl NetworkRequest;

typedef enum {
  NETWORK_REQUEST_METHOD_GET,
  NETWORK_REQUEST_METHOD_POST,
  NETWORK_REQUEST_METHOD_PUT,
  NETWORK_REQUEST_METHOD_DELETE,
} NetworkRequestMethod;

CORE_EXTERN_C_BEGIN

CORE_PUBLIC NetworkRequest* core_network_request_new(const char* url, NetworkRequestMethod method);

CORE_PUBLIC void core_network_request_add_header(NetworkRequest* request, const char* value);

CORE_PUBLIC void core_network_request_add_content(NetworkRequest* request, const char* content);

CORE_PUBLIC void core_network_request_delete(const NetworkRequest* request);

CORE_EXTERN_C_END

#endif // GAME_MAPS_CORE_NETWORK_REQUEST_H