#include <string>
#include "data.h"
#include "leveldb.h"
#include <iostream>
#include <filesystem>
#include <sstream>
#include <fstream>

#ifndef GAME_MAPS_DB_PATH
    #define GAME_MAPS_DB_PATH "game-maps-db-v2"
#endif // GAME_MAPS_DB_PATH

Data* Data::start() {
    return newLevelDB(GAME_MAPS_DB_PATH);
}