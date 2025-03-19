#ifndef GAME_MAPS_CORE_NETWORK_H
#define GAME_MAPS_CORE_NETWORK_H

#include "stdafx.h"

struct NetworkRequestImpl;

typedef REMOVE_IN_CPP(struct) NetworkRequestImpl NetworkRequest;

typedef enum {
  NETWORK_REQUEST_CONTENT_TYPE_JSON,
} NetworkRequestContentType;

typedef enum {
  NETWORK_REQUEST_METHOD_GET,
  NETWORK_REQUEST_METHOD_POST,
  NETWORK_REQUEST_METHOD_PUT,
  NETWORK_REQUEST_METHOD_DELETE,
} NetworkRequestMethod;

typedef enum {
  NETWORK_RESPONSE_STATUS_OK = 200,
  NETWORK_RESPONSE_STATUS_NO_CONTENT = 204,
  NETWORK_RESPONSE_STATUS_NOT_FOUND = 404,
  NETWORK_RESPONSE_STATUS_METHOD_NOT_ALLOWED = 405,
} NetworkResponseStatus;

typedef struct {
  NetworkResponseStatus status;
} NetworkResponse;

CORE_EXTERN_C_BEGIN

CORE_PUBLIC NetworkRequest* core_network_request_new(const char* url, NetworkRequestMethod method);

CORE_PUBLIC void core_network_request_add_header(NetworkRequest* request, const char* header);

CORE_PUBLIC void core_network_request_set_content(NetworkRequest* request, const char* content, NetworkRequestContentType content_type);

CORE_PUBLIC void core_network_request_delete(const NetworkRequest* request);

CORE_PUBLIC NetworkResponse* core_network_override(const NetworkRequest* request);

CORE_PUBLIC void core_network_response_delete(NetworkResponse* response);

CORE_EXTERN_C_END

#endif // GAME_MAPS_CORE_NETWORK_H