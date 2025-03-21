#ifndef GAME_MAPS_CORE_DATA_LEVELDB_HPP
#define GAME_MAPS_CORE_DATA_LEVELDB_HPP

#include "data.hpp"
#include <leveldb/db.h>

struct DataLevelDB final : Data {
  DataLevelDB(leveldb::DB* ldb);
  ~DataLevelDB();

  bool write(std::string key, std::string value) override;
  bool read(std::string key, std::string &value) override;

  leveldb::DB* db;
};

DataLevelDB* newLevelDB(const char* db_path);

#endif // GAME_MAPS_CORE_DATA_LEVELDB_HPP