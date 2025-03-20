#include "mapgenie-network.hpp"
#include <catch2/catch_test_macros.hpp>
#include <ylt/easylog.hpp>

TEST_CASE("Test edit map html", "[core][mapgenie][edit][html]") {
  std::ifstream file(MAPGENIE_INJECT_HTML_PATH);

  std::stringstream buffer;
  buffer << file.rdbuf();
  file.close();

  NetworkResponse response {
    .status = NETWORK_RESPONSE_STATUS_OK,
    // ReSharper disable once CppDeprecatedEntity
    .content = strdup(buffer.str().c_str()),
    .headers = nullptr,
  };

  edit_map_html(&response);

  std::ofstream file_out(MAPGENIE_INJECT_HTML_PATH_OUT);
  file_out << response.content;
  file_out.close();
}