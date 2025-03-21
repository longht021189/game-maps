#ifndef GAME_MAPS_CORE_NETWORK_RESPONSE_H
#define GAME_MAPS_CORE_NETWORK_RESPONSE_H

#include "stdafx.h"

typedef enum {
  NETWORK_RESPONSE_STATUS_OK = 200,
  NETWORK_RESPONSE_STATUS_NO_CONTENT = 204,
  NETWORK_RESPONSE_STATUS_NOT_FOUND = 404,
  NETWORK_RESPONSE_STATUS_METHOD_NOT_ALLOWED = 405,
} NetworkResponseStatus;

typedef struct {
  NetworkResponseStatus status;
  char* content;
  char* headers;
} NetworkResponse;

CORE_EXTERN_C_BEGIN

CORE_PUBLIC void core_network_response_delete(NetworkResponse* response);

CORE_EXTERN_C_END

#endif // GAME_MAPS_CORE_NETWORK_RESPONSE_H