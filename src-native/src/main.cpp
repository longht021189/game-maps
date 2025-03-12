#include <windows.h>
#include <iostream>
#include <thread>
#include <string>
#include "mapgenie-data.h"
#include "keyboard.h"

Data *database = nullptr;
MapgenieData *mapgenie_data = nullptr;
Keyboard *keyboard = nullptr;

extern "C" void core_start() {
    if (database == nullptr) {
        database = Data::start();
    }
    if (database != nullptr && mapgenie_data == nullptr) {
        mapgenie_data = new MapgenieData(database);
    }
    if (keyboard == nullptr) {
        keyboard = Keyboard::start();
    }
}

extern "C" void core_end() {
    if (mapgenie_data != nullptr) {
        delete mapgenie_data;
        mapgenie_data = nullptr;
    }
    if (database != nullptr) {
        delete database;
        database = nullptr;
    }
    if (keyboard != nullptr) {
        delete keyboard;
        keyboard = nullptr;
    }
}

extern "C" void mapgenie_add_location(uint64_t game_id, uint64_t map_id, uint64_t location_id) {
    if (mapgenie_data == nullptr) {
        return;
    }

    mapgenie_data->add_location(game_id, map_id, location_id);
}

extern "C" void mapgenie_remove_location(uint64_t game_id, uint64_t map_id, uint64_t location_id) {
    if (mapgenie_data == nullptr) {
        return;
    }

    mapgenie_data->remove_location(game_id, map_id, location_id);
}

extern "C" char* mapgenie_get_map_data(uint64_t game_id, uint64_t map_id, size_t* length) {
    if (mapgenie_data == nullptr) {
        return nullptr;
    }

    std::string output;
    mapgenie_data->get_map_data(game_id, map_id, output);
    char* buffer = (char*)malloc(output.size() + 1);
    if (buffer) {
        *length = output.size();
        std::memcpy(buffer, output.c_str(), output.size() + 1);
    }
    return buffer;
}

extern "C" void free_buffer(char* ptr) {
    free(ptr);
}