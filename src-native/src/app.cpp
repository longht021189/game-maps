#include "app.hpp"

#ifndef MAPGENIE_RESOURCES_PATH
  #define MAPGENIE_RESOURCES_PATH "_up_/dist"
#endif

std::string mapgenie_resources_path_value = MAPGENIE_RESOURCES_PATH;

CORE_EXTERN_C CORE_PUBLIC void core_app_config(const AppConfig config) {
  mapgenie_resources_path_value = config.mapgenie_resources_path;
}

CORE_PRIVATE const std::string& mapgenie_resources_path() {
  return mapgenie_resources_path_value;
}