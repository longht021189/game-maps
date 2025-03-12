cmake_minimum_required(VERSION 3.10)

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

project(game-maps-core)

# Set C++ standard to 11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Optional: Ensures standard compliance (no compiler extensions)

# Add executable
add_library(game-maps-core STATIC src/main.cpp src/data.cpp src/mapgenie-data.cpp)

# Link with libs
find_package(CURL REQUIRED)
find_package(leveldb CONFIG REQUIRED)
find_package(yalantinglibs CONFIG REQUIRED)
target_link_libraries(game-maps-core PRIVATE CURL::libcurl leveldb::leveldb yalantinglibs::yalantinglibs)

# Link against Windows libraries
target_link_libraries(game-maps-core PRIVATE user32)
