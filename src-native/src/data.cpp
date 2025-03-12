#include <string>
#include "data.h"
#include <leveldb/db.h>
#include <iostream>

Data::Data() {
}

Data::~Data() {
    delete db;
}

bool Data::init() {
    if (db != nullptr) {
        return true;
    }

    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, "game-maps-db", &db);

    if (!status.ok()) {
        std::cerr << "Unable to open/create database: " << status.ToString() << std::endl;
        return false;
    }
    return true;
}

bool Data::write(std::string key, std::string value) {
    if (db == nullptr) {
        return false;
    }

    leveldb::Status status = db->Put(leveldb::WriteOptions(), key.c_str(), value.c_str());
    return status.ok();
}

bool Data::read(std::string key, std::string &value) {
    if (db == nullptr) {
        return false;
    }

    leveldb::Status status = db->Get(leveldb::ReadOptions(), key.c_str(), &value);
    return status.ok();
}