#include <string>
#include "keyboard.h"
#include <catch2/catch_test_macros.hpp>
#include <thread>

TEST_CASE("Close Keyboard", "[keyboard][close]") {
    auto keyboard = Keyboard::start();
    REQUIRE_FALSE(keyboard == nullptr);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    delete keyboard;
}