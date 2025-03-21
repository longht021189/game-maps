#include "mapgenie-network.hpp"
#include "network-request.hpp"
#include "mapgenie-data.hpp"
#include "network-response.hpp"
#include "network-fetch.hpp"
#include "utils.hpp"
#include "app.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

MapgenieData mapgenie_data{};

std::vector<std::string> mapgenie_network_filters = {
  "https://mapgenie.io/*/maps/*",
  "https://mapgenie.io/static/*",
  "https://mapgenie.io/api/local/map-data/*",
  "https://mapgenie.io/api/v1/user/notes",
  "https://mapgenie.io/api/v1/user/locations/*"
};

struct MapgenieHeadersData {
  uint64_t game_id;
  uint64_t map_id;
};

CORE_PRIVATE void get_scripts(
  const std::string& directory,
  const std::string& ext,
  std::string& output
) {
  for (const auto& entry : fs::recursive_directory_iterator(directory)) {
    if (entry.is_regular_file()) {
      std::string filename = entry.path().filename().string();
      if (
        std::string extension = entry.path().extension().string();
        extension == ext
      ) {
        output += "<script src=\"/static/js/" + filename + "\"></script>\n";
      }
    }
  }
}

CORE_PRIVATE void get_links(
  const std::string& directory,
  const std::string& ext,
  std::string& output
) {
  for (const auto& entry : fs::recursive_directory_iterator(directory)) {
    if (entry.is_regular_file()) {
      std::string filename = entry.path().filename().string();
      if (
        std::string extension = entry.path().extension().string();
        extension == ext
      ) {
        output += "<link href=\"/static/css/" + filename + "\" rel=\"stylesheet\">\n";
      }
    }
  }
}

CORE_PRIVATE void get_headers_data(const NetworkRequest* request, MapgenieHeadersData& output) {
  const curl_slist *header = request->m_headers;
  while (nullptr != header) {
    if (starts_with(header->data, "X-Map-ID") == 1) {
      output.map_id = string_to_uint64_ss(header->data + 9);
    }
    if (starts_with(header->data, "X-Game-ID") == 1) {
      output.game_id = string_to_uint64_ss(header->data + 10);
    }
    header = header->next;
  }
}

CORE_PRIVATE void inject_sources(std::string& content) {
  auto const js_dir = mapgenie_resources_path() + "/static/js";
  auto const css_dir = mapgenie_resources_path() + "/static/css";

  std::string data;
  get_scripts(js_dir, ".js", data);
  get_links(css_dir, ".css", data);

  const auto start = content.find("</head>");
  replace_range(content, "</head>", data + "</head>", start, start + 20);
}

CORE_PRIVATE void edit_map_html(NetworkResponse* response) {
  auto content = std::string(response->content);
  inject_sources(content);

  const auto start = content.find("<script>window.mapUrl");
  const auto end = content.find("</script>", start);
  replace_range(content, "</script>", "injectInputs();</script>", end, end + 20);

  free(response->content);

  // ReSharper disable once CppDeprecatedEntity
  response->content = strdup(content.c_str());
}

CORE_PRIVATE const char* get_content_type_from_extension(const std::string& filename) {
  const size_t dot_pos = filename.rfind('.');
  if (dot_pos == std::string::npos) {
    return "Content-Type: unknown";
  }

  if (
    const std::string ext = filename.substr(dot_pos);
    ext == ".css"
  ) {
    return "Content-Type: text/css";
  } else if (ext == ".js") {
    return "Content-Type: application/javascript";
  } else {
    return "Content-Type: unknown";
  }
}

CORE_PRIVATE NetworkResponse* core_mapgenie_network_override(const NetworkRequest* request) {
  if (starts_with(request->m_parsed_url->path, "/api/v1/user/locations/") == 1) {
    MapgenieHeadersData headers_data {};
    get_headers_data(request, headers_data);

    const auto parts = split(request->m_parsed_url->path, '/');
    const auto location_id = string_to_uint64_ss(parts[parts.size() - 1]);

    switch (request->m_method) {
      case NETWORK_REQUEST_METHOD_PUT:
        mapgenie_data.add_location(
          headers_data.game_id,
          headers_data.map_id,
          location_id
        );

        return new NetworkResponse{
          .status = NETWORK_RESPONSE_STATUS_OK,
          // ReSharper disable once CppDeprecatedEntity
          .content = strdup("{}"),
          // ReSharper disable once CppDeprecatedEntity
          .headers = strdup("Content-Type: application/json; charset=utf-8"),
        };

      case NETWORK_REQUEST_METHOD_DELETE:
        mapgenie_data.remove_location(
          headers_data.game_id,
          headers_data.map_id,
          location_id
        );

        return new NetworkResponse{
          .status = NETWORK_RESPONSE_STATUS_NO_CONTENT,
          .content = nullptr,
          .headers = nullptr,
        };

      default:
        return &NetworkResponseErrors::get()->error_method_not_allowed;
    }
  }
  if (request->m_parsed_url->path == "/api/v1/user/notes") {
    MapgenieHeadersData headers_data {};
    get_headers_data(request, headers_data);

    switch (request->m_method) {
      case NETWORK_REQUEST_METHOD_POST: {
        std::string output;

        mapgenie_data.add_note(
          headers_data.game_id,
          headers_data.map_id,
          request->m_content,
          output
        );

        return new NetworkResponse{
          .status = NETWORK_RESPONSE_STATUS_OK,
          // ReSharper disable once CppDeprecatedEntity
          .content = strdup(output.c_str()),
          // ReSharper disable once CppDeprecatedEntity
          .headers = strdup("Content-Type: application/json; charset=utf-8"),
        };
      }

      case NETWORK_REQUEST_METHOD_PUT: {
        const auto parts = split(request->m_parsed_url->path, '/');
        const auto& note_id = parts[parts.size() - 1];

        std::string output;

        mapgenie_data.update_note(
          headers_data.game_id,
          headers_data.map_id,
          note_id,
          request->m_content,
          output
        );

        return new NetworkResponse{
          .status = NETWORK_RESPONSE_STATUS_OK,
          // ReSharper disable once CppDeprecatedEntity
          .content = strdup(output.c_str()),
          // ReSharper disable once CppDeprecatedEntity
          .headers = strdup("Content-Type: application/json; charset=utf-8"),
        };
      }

      case NETWORK_REQUEST_METHOD_DELETE: {
        const auto parts = split(request->m_parsed_url->path, '/');
        const auto& note_id = parts[parts.size() - 1];

        mapgenie_data.delete_note(
          headers_data.game_id,
          headers_data.map_id,
          note_id
        );

        return new NetworkResponse{
          .status = NETWORK_RESPONSE_STATUS_NO_CONTENT,
          .content = nullptr,
          .headers = nullptr,
        };
      }

      default:
        return &NetworkResponseErrors::get()->error_method_not_allowed;
    }
  }
  if (starts_with(request->m_parsed_url->path, "/api/local/map-data/") == 1) {
    const auto parts = split(request->m_parsed_url->path, '/');
    const auto game_id = string_to_uint64_ss(parts[parts.size() - 2]);
    const auto map_id = string_to_uint64_ss(parts[parts.size() - 1]);

    std::string output;
    mapgenie_data.get_map_data(game_id, map_id, output);

    if (request->m_method == NETWORK_REQUEST_METHOD_GET) {
      return new NetworkResponse{
        .status = NETWORK_RESPONSE_STATUS_OK,
        // ReSharper disable once CppDeprecatedEntity
        .content = strdup(output.c_str()),
        // ReSharper disable once CppDeprecatedEntity
        .headers = strdup("Content-Type: application/json; charset=utf-8"),
      };
    }

    return &NetworkResponseErrors::get()->error_method_not_allowed;
  }
  if (starts_with(request->m_parsed_url->path, "/static/") == 1) {
    const auto filepath = mapgenie_resources_path() + request->m_parsed_url->path;

    std::ifstream file(filepath);
    if (!file.is_open()) {
      return &NetworkResponseErrors::get()->error_not_found;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return new NetworkResponse{
      .status = NETWORK_RESPONSE_STATUS_OK,
      // ReSharper disable once CppDeprecatedEntity
      .content = strdup(buffer.str().c_str()),
      // ReSharper disable once CppDeprecatedEntity
      .headers = strdup(get_content_type_from_extension(request->m_parsed_url->path)),
    };
  }
  if (starts_with(request->m_url, "https://mapgenie.io/") == 1) {
    if (
      const auto parts = split(request->m_parsed_url->path, '/');
      parts.size() > 1 && parts[1] == "maps"
    ) {
      const auto response = core_network_fetch(request);
      if (response->status != NETWORK_RESPONSE_STATUS_OK) {
        return response;
      }

      edit_map_html(response);
      return response;
    }
  }

  return nullptr;
}

CORE_PRIVATE const std::vector<std::string>& core_mapgenie_network_filters() {
  return mapgenie_network_filters;
}