#include <string>
#include "data.h"
#include <leveldb/db.h>
#include <iostream>

#ifndef GAME_MAPS_DB_PATH
    #define GAME_MAPS_DB_PATH "game-maps-db"
#endif // GAME_MAPS_DB_PATH

Data::~Data() {
    delete db;
}

bool Data::write(std::string key, std::string value) {
    leveldb::Status status = db->Put(leveldb::WriteOptions(), key.c_str(), value.c_str());
    return status.ok();
}

bool Data::read(std::string key, std::string &value) {
    leveldb::Status status = db->Get(leveldb::ReadOptions(), key.c_str(), &value);
    return status.ok();
}

Data* Data::start() {
    leveldb::Options options;
    options.create_if_missing = true;

    leveldb::DB* db = nullptr;
    leveldb::Status status = leveldb::DB::Open(options, GAME_MAPS_DB_PATH, &db);

    if (!status.ok()) {
        std::cout << "[Data::start] Unable to open/create database: " << status.ToString() << std::endl;
        return nullptr;
    }

    Data *data = new Data();
    data->db = db;
    return data;
}