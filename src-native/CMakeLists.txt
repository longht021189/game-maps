cmake_minimum_required(VERSION 3.10)

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

project(game-maps-core)

# Set C++ standard to 11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Optional: Ensures standard compliance (no compiler extensions)

# Target
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

if (GAME_MAPS_CORE_RELEASE)
    add_library(game-maps-core SHARED ${SOURCES})
    target_compile_definitions(game-maps-core PRIVATE GAME_MAPS_CORE_RELEASE)
else ()
    file(GLOB_RECURSE SOURCES_TESTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)
    add_executable(game-maps-core ${SOURCES} ${SOURCES_TESTS})
    find_package(Catch2 CONFIG REQUIRED)
    target_link_libraries(game-maps-core PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)
    target_compile_definitions(game-maps-core PRIVATE GAME_MAPS_DB_PATH="${CMAKE_CURRENT_SOURCE_DIR}/game-maps-db")
endif ()

# Features
target_compile_features(game-maps-core PRIVATE cxx_std_20)

# Includes
target_include_directories(game-maps-core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Link with libs
find_package(CURL REQUIRED)
find_package(leveldb CONFIG REQUIRED)
find_package(yalantinglibs CONFIG REQUIRED)
target_link_libraries(game-maps-core PRIVATE CURL::libcurl leveldb::leveldb yalantinglibs::yalantinglibs)

# Link against Windows libraries
target_link_libraries(game-maps-core PRIVATE user32)
