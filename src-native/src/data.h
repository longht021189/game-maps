#ifndef GAME_MAPS_CORE_DATA_H
#define GAME_MAPS_CORE_DATA_H

namespace leveldb {
    class DB;
}

struct Data {
    Data();
    virtual ~Data();

    bool init();
    bool write(std::string key, std::string value);
    bool read(std::string key, std::string &value);

private:
    leveldb::DB* db = nullptr;
};

#endif // GAME_MAPS_CORE_DATA_H