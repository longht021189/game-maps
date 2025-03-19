#ifndef GAME_MAPS_CORE_MAIN_H
#define GAME_MAPS_CORE_MAIN_H

struct curl_slist;

#ifdef __cplusplus
    typedef curl_slist Header;
#else
    typedef struct curl_slist Header;
#endif

typedef enum {
    REQUEST_METHOD_GET,
    REQUEST_METHOD_POST,
    REQUEST_METHOD_PUT,
    REQUEST_METHOD_DELETE,
} RequestMethod;

typedef enum {
    REQUEST_CONTENT_TYPE_JSON,
} RequestContentType;

typedef struct {
    const char* url;
    RequestMethod method;
    RequestContentType content_type;
    const char* content;
    Header* headers;
} Request;

typedef enum {
    Code_200_Success            = 200,
    Code_204_NoContent          = 204,
    Code_404_NotFound           = 404,
    Code_405_MethodNotAllowed   = 405,
} ResponseStatus;

typedef struct {
    ResponseStatus status;
} Response;

#ifndef EXPORT
    #define EXPORT
#endif // EXPORT

#ifdef __cplusplus
extern "C" {
#endif

EXPORT void add_header(Request* request, const char* value);

EXPORT void free_headers(Request* request);

EXPORT Response* fetch(Request* request);

EXPORT void free_response(Response* response);

#ifdef __cplusplus
}
#endif

#endif // GAME_MAPS_CORE_MAIN_H