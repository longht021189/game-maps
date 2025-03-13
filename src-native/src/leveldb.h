#ifndef GAME_MAPS_CORE_LEVELDB_H
#define GAME_MAPS_CORE_LEVELDB_H

#include <string>
#include "data.h"
#include <leveldb/db.h>

struct LevelDB final : Data {
    LevelDB(leveldb::DB* ldb);
    ~LevelDB();

    bool write(std::string key, std::string value) override;
    bool read(std::string key, std::string &value) override;

    leveldb::DB* db;
};

LevelDB* newLevelDB(const char* db_path);

#endif // GAME_MAPS_CORE_LEVELDB_H