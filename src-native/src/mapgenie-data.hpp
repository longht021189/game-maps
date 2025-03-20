#ifndef GAME_MAPS_CORE_MAPGENIE_DATA_H
#define GAME_MAPS_CORE_MAPGENIE_DATA_H

#include <cstdint>
#include "data.hpp"

struct MapgenieMapData;

struct MapgenieData {
    MapgenieData();

    virtual ~MapgenieData() = default;

    void add_note(uint64_t game_id, uint64_t map_id, const std::string& note, std::string& output) const;
    void update_note(uint64_t game_id, uint64_t map_id, std::string note_id, const std::string& note, std::string& output) const;
    void delete_note(uint64_t game_id, uint64_t map_id, std::string note_id) const;
    void add_location(uint64_t game_id, uint64_t map_id, uint64_t location_id) const;
    void remove_location(uint64_t game_id, uint64_t map_id, uint64_t location_id) const;
    void get_map_data(uint64_t game_id, uint64_t map_id, std::string& output) const;

private:
    void get_data(uint64_t game_id, uint64_t map_id, MapgenieMapData &value) const;
    void set_data(uint64_t game_id, uint64_t map_id, MapgenieMapData &value) const;

    Data* data;
};

#endif // GAME_MAPS_CORE_MAPGENIE_DATA_H