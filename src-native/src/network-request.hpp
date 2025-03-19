#ifndef GAME_MAPS_CORE_NETWORK_REQUEST_HPP
#define GAME_MAPS_CORE_NETWORK_REQUEST_HPP

typedef REMOVE_IN_CPP(struct) curl_slist Header;

struct NetworkRequestImpl final {
  Header* m_headers = nullptr;
  char* m_content = nullptr;
  NetworkRequestContentType m_content_type = NETWORK_REQUEST_CONTENT_TYPE_JSON;
  std::string m_url;
  NetworkRequestMethod m_method;
  ParsedURL* m_parsed_url;

  NetworkRequestImpl(const char* url, ParsedURL* parsed_url, const NetworkRequestMethod method)
    : m_url(url)
    , m_method(method)
    , m_parsed_url(parsed_url)
  {}

  ~NetworkRequestImpl() {
    delete[] m_content;
    delete m_parsed_url;

    if (m_headers) {
      curl_slist_free_all(m_headers);
    }
  }
};

#endif // GAME_MAPS_CORE_NETWORK_REQUEST_HPP