#include <string>
#include "data.h"
#include "leveldb.h"
#include <iostream>
#include <filesystem>
#include <sstream>
#include <fstream>

#ifndef GAME_MAPS_DB_PATH
    #define GAME_MAPS_DB_PATH "game-maps-db"
#endif // GAME_MAPS_DB_PATH

namespace fs = std::filesystem;

struct Filesystem final : Data {
    Filesystem(const char* path): file_dir(path) {}

    bool write(std::string key, std::string value) override {
        if (!fs::exists(file_dir)) {
            try {
                fs::create_directories(file_dir);
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error creating folder: " << e.what() << std::endl;
                return false;
            }
        }

        std::stringstream ss;
        ss << file_dir << "/" << key << ".txt";

        std::ofstream outFile(ss.str());
        if (outFile.is_open()) {
            outFile << value;
            outFile.close();
            return true;
        } else {
            std::cerr << "Error: Unable to open file for writing" << std::endl;
            return false;
        }
    }

    bool read(std::string key, std::string &value) override {
        std::stringstream ss;
        ss << file_dir << "/" << key << ".txt";

        std::ifstream inFile(ss.str());
        if (inFile.is_open()) {
            std::string line;
            bool first = true;
            while (std::getline(inFile, line)) {
                if (!first) {
                    value += "\n";
                }

                first = false;
                value += line;
            }
            inFile.close();
            return true;
        } else {
            std::cerr << "Key " << key << " does not exist or cannot be opened" << std::endl;
            return false;
        }
    }

    std::string file_dir;
};

Data* Data::start() {
    // return newLevelDB(GAME_MAPS_DB_PATH);
    return new Filesystem(GAME_MAPS_DB_PATH);
}