#include <string>
#include "mapgenie-data.h"
#include <ylt/struct_json/json_writer.h>
#include <ylt/struct_json/json_reader.h>
#include <vector>
#include <sstream>

struct MapgenieMapData {
    std::vector<uint64_t> locations {};
    uint64_t game_id = 0;
    uint64_t map_id = 0;
};

YLT_REFL(MapgenieMapData, locations, game_id, map_id);

MapgenieData::MapgenieData(Data *d)
    : data(d)
{}

MapgenieData::~MapgenieData() {}

void MapgenieData::get_data(
    uint64_t game_id, 
    uint64_t map_id, 
    MapgenieMapData &value
) {
    std::stringstream ss{};
    ss << "mapgenie_mapdata_" << game_id << "_" << map_id;

    std::string text;
    if (data->read(ss.str(), text)) {
        std::error_code error;
        struct_json::from_json(value, text, error);
        if (error.value() != 0) {
            value.game_id = game_id;
            value.map_id = map_id;
            value.locations.clear();
        }
    } else {
        value.game_id = game_id;
        value.map_id = map_id;
        value.locations.clear();
    }
}

void MapgenieData::set_data(
    uint64_t game_id, 
    uint64_t map_id, 
    MapgenieMapData &value
) {
    std::stringstream ss{};
    ss << "mapgenie_mapdata_" << game_id << "_" << map_id;

    std::string text;
    struct_json::to_json(value, text);
    data->write(ss.str(), text);
}

void MapgenieData::add_location(
    uint64_t game_id, 
    uint64_t map_id, 
    uint64_t location_id
) {
    MapgenieMapData value;
    get_data(game_id, map_id, value);
    value.locations.push_back(location_id);
    set_data(game_id, map_id, value);
}

void MapgenieData::remove_location(
    uint64_t game_id, 
    uint64_t map_id, 
    uint64_t location_id
) {
    MapgenieMapData value;
    get_data(game_id, map_id, value);
    value.locations.erase(
        std::remove(value.locations.begin(), value.locations.end(), location_id), 
        value.locations.end());
    set_data(game_id, map_id, value);
}

void MapgenieData::get_map_data(
    uint64_t game_id, 
    uint64_t map_id, 
    std::string& output
) {
    MapgenieMapData value;
    get_data(game_id, map_id, value);
    struct_json::to_json(value, output);
}