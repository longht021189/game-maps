#include <windows.h>
#include <iostream>
#include <thread>
#include <string>
#include "mapgenie-data.h"
#include "keyboard.h"

#define EXPORT __declspec(dllexport)
#include "main.h"

Data *database = nullptr;
MapgenieData *mapgenie_data = nullptr;
Keyboard *keyboard = nullptr;

extern "C" EXPORT void core_start() {
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

extern "C" EXPORT void core_end() {
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

extern "C" EXPORT void mapgenie_add_location(uint64_t game_id, uint64_t map_id, uint64_t location_id) {
    if (mapgenie_data == nullptr) {
        return;
    }

    mapgenie_data->add_location(game_id, map_id, location_id);
}

extern "C" EXPORT void mapgenie_remove_location(uint64_t game_id, uint64_t map_id, uint64_t location_id) {
    if (mapgenie_data == nullptr) {
        return;
    }

    mapgenie_data->remove_location(game_id, map_id, location_id);
}

extern "C" EXPORT char* mapgenie_get_map_data(uint64_t game_id, uint64_t map_id, size_t* length) {
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

extern "C" EXPORT void free_buffer(char* ptr) {
    free(ptr);
}

extern "C" EXPORT char* mapgenie_add_note(
    uint64_t game_id, 
    uint64_t map_id, 
    const char* body, 
    int32_t body_len
) {
    if (mapgenie_data == nullptr) {
        return nullptr;
    }

    std::string output;
    std::string note(body, body_len);

    mapgenie_data->add_note(game_id, map_id, note, output);

    char* buffer = (char*)malloc(output.size() + 1);
    if (buffer) {
        std::memcpy(buffer, output.c_str(), output.size() + 1);
    }
    return buffer;
}

extern "C" EXPORT char* mapgenie_update_note(
    uint64_t game_id, 
    uint64_t map_id, 
    const char* body, 
    int32_t body_len,
    const char* note_id, 
    int32_t note_id_len
) {
    if (mapgenie_data == nullptr) {
        return nullptr;
    }

    std::string output;
    std::string note(body, body_len);
    std::string note_id_str(note_id, note_id_len);

    mapgenie_data->update_note(game_id, map_id, note_id_str, note, output);
    
    char* buffer = (char*)malloc(output.size() + 1);
    if (buffer) {
        std::memcpy(buffer, output.c_str(), output.size() + 1);
    }
    return buffer;
}

extern "C" EXPORT void mapgenie_delete_note(
    uint64_t game_id,
    uint64_t map_id,
    const char* note_id, 
    int32_t note_id_len
) {
    if (mapgenie_data == nullptr) {
        return;
    }

    std::string note_id_str(note_id, note_id_len);
    mapgenie_data->delete_note(game_id, map_id, note_id_str);
}
