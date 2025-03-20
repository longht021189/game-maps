#include "data-leveldb.hpp"

DataLevelDB::DataLevelDB(leveldb::DB* ldb): db(ldb) {
}

DataLevelDB::~DataLevelDB() {
  delete db;
}

bool DataLevelDB::write(std::string key, std::string value) {
  leveldb::Status status = db->Put(leveldb::WriteOptions(), key.c_str(), value.c_str());
  return status.ok();
}

bool DataLevelDB::read(std::string key, std::string &value) {
  leveldb::Status status = db->Get(leveldb::ReadOptions(), key.c_str(), &value);
  return status.ok();
}

DataLevelDB* newLevelDB(const char* db_path) {
  leveldb::Options options;
  options.create_if_missing = true;

  leveldb::DB* db = nullptr;
  leveldb::Status status = leveldb::DB::Open(options, db_path, &db);

  if (!status.ok()) {
    // std::cout << "[Data::start] Unable to open/create database: " << status.ToString() << std::endl;
    return nullptr;
  }

  return new DataLevelDB(db);
}