#include <string>
#include "mapgenie-data.h"
#include <ylt/struct_json/json_writer.h>
#include <ylt/struct_json/json_reader.h>
#include <vector>
#include <sstream>
#include <random>

struct MapgenieMapNoteInput {
    uint64_t map_id;
    std::string title;
    std::string description;
    std::optional<std::string> color;
    double latitude;
    double longitude;
};

struct MapgenieMapNote : MapgenieMapNoteInput {
    std::string id;
    std::string created_at;
    uint64_t user_id;
};

struct MapgenieMapData {
    std::vector<uint64_t> locations {};
    std::vector<MapgenieMapNote> notes {};
    uint64_t game_id = 0;
    uint64_t map_id = 0;
};

YLT_REFL(MapgenieMapNoteInput, map_id, title, description, color, latitude, longitude);
YLT_REFL(MapgenieMapNote, map_id, title, description, color, latitude, longitude, id, created_at, user_id);
YLT_REFL(MapgenieMapData, locations, game_id, map_id, notes);

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

std::string generate_random_id(int count) {
    // Character sets
    const std::string character_sets = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    // Random number generation setup
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> sets_dist(0, character_sets.size() - 1);

    // Random
    std::string id;
    for (int i = 0; i < count; ++i) {
        id += character_sets[sets_dist(gen)];
    }
    return id;
}

void MapgenieData::add_note(
    uint64_t game_id, 
    uint64_t map_id, 
    std::string note, 
    std::string& output
) {
    MapgenieMapNoteInput input;
    struct_json::from_json(input, note);

    MapgenieMapNote note_data;
    note_data.map_id = input.map_id;
    note_data.title = input.title;
    note_data.description = input.description;
    note_data.color = input.color;
    note_data.latitude = input.latitude;
    note_data.longitude = input.longitude;
    note_data.created_at = "2025-03-13T02:19:15.000000Z";
    note_data.id = generate_random_id(20);
    note_data.user_id = 123456;

    MapgenieMapData mapdata;
    get_data(game_id, map_id, mapdata);
    mapdata.notes.push_back(note_data);
    set_data(game_id, map_id, mapdata);

    struct_json::to_json(note_data, output);
}

void MapgenieData::update_note(
    uint64_t game_id, 
    uint64_t map_id, 
    std::string note_id, 
    std::string note, 
    std::string& output
) {
    MapgenieMapData mapdata;
    get_data(game_id, map_id, mapdata);

    auto it = std::find_if(mapdata.notes.begin(), mapdata.notes.end(),
        [&note_id](const MapgenieMapNote& p) { return p.id == note_id; });

    if (it != mapdata.notes.end()) {
        mapdata.notes.erase(it);
    }

    MapgenieMapNoteInput input;
    struct_json::from_json(input, note);

    MapgenieMapNote note_data;
    note_data.map_id = input.map_id;
    note_data.title = input.title;
    note_data.description = input.description;
    note_data.color = input.color;
    note_data.latitude = input.latitude;
    note_data.longitude = input.longitude;
    note_data.created_at = "2025-03-13T02:19:15.000000Z";
    note_data.id = note_id;
    note_data.user_id = 123456;

    mapdata.notes.push_back(note_data);
    set_data(game_id, map_id, mapdata);

    struct_json::to_json(note_data, output);
}

void MapgenieData::delete_note(
    uint64_t game_id, 
    uint64_t map_id, 
    std::string note_id
) {
    MapgenieMapData mapdata;
    get_data(game_id, map_id, mapdata);

    auto it = std::find_if(mapdata.notes.begin(), mapdata.notes.end(),
        [&note_id](const MapgenieMapNote& p) { return p.id == note_id; });

    if (it != mapdata.notes.end()) {
        mapdata.notes.erase(it);
        set_data(game_id, map_id, mapdata);
    }
}