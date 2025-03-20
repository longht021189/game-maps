#include "data.hpp"
#include "data-leveldb.hpp"

#ifndef GAME_MAPS_DB_PATH
    #define GAME_MAPS_DB_PATH "game-maps-db-v2"
#endif // GAME_MAPS_DB_PATH

Data* Data::instance = nullptr;

Data* Data::get_instance() {
  if (instance) {
    return instance;
  }

  instance = newLevelDB(GAME_MAPS_DB_PATH);
  return instance;
}