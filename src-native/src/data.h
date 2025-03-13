#ifndef GAME_MAPS_CORE_DATA_H
#define GAME_MAPS_CORE_DATA_H

struct Data {
    static Data* start();

    virtual ~Data() {};

    virtual bool write(std::string key, std::string value) = 0;
    virtual bool read(std::string key, std::string &value) = 0;
};

#endif // GAME_MAPS_CORE_DATA_H