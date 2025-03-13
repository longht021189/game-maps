#include "leveldb.h"
#include <iostream>

LevelDB::LevelDB(leveldb::DB* ldb): db(ldb) {
}

LevelDB::~LevelDB() {
    delete db;
}

bool LevelDB::write(std::string key, std::string value) {
    leveldb::Status status = db->Put(leveldb::WriteOptions(), key.c_str(), value.c_str());
    return status.ok();
}

bool LevelDB::read(std::string key, std::string &value) {
    leveldb::Status status = db->Get(leveldb::ReadOptions(), key.c_str(), &value);
    return status.ok();
}

LevelDB* newLevelDB(const char* db_path) {
    leveldb::Options options;
    options.create_if_missing = true;

    leveldb::DB* db = nullptr;
    leveldb::Status status = leveldb::DB::Open(options, db_path, &db);

    if (!status.ok()) {
        std::cout << "[Data::start] Unable to open/create database: " << status.ToString() << std::endl;
        return nullptr;
    }

    return new LevelDB(db);
}