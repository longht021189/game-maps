#ifndef GAME_MAPS_CORE_MAPGENIE_DATA_H
#define GAME_MAPS_CORE_MAPGENIE_DATA_H

#include <cstdint>
#include "data.h"

struct MapgenieMapData;

struct MapgenieData {
    MapgenieData(Data* d);
    virtual ~MapgenieData();

    void add_location(uint64_t game_id, uint64_t map_id, uint64_t location_id);
    void remove_location(uint64_t game_id, uint64_t map_id, uint64_t location_id);
    void get_map_data(uint64_t game_id, uint64_t map_id, std::string& output);

private:
    void get_data(uint64_t game_id, uint64_t map_id, MapgenieMapData &value);
    void set_data(uint64_t game_id, uint64_t map_id, MapgenieMapData &value);

    Data* data;
};

#endif // GAME_MAPS_CORE_MAPGENIE_DATA_H