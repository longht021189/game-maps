#include <string>
#include "data.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Check Data", "[data]") {
    Data* data = Data::start();
    REQUIRE_FALSE(data == nullptr);

    bool result = data->write("test", "test-1234");
    REQUIRE(result == true);

    std::string value;
    result = data->read("test", value);
    REQUIRE(result == true);
    REQUIRE(value == "test-1234");

    delete data;

    data = Data::start();
    REQUIRE_FALSE(data == nullptr);

    result = data->read("test", value);
    REQUIRE(result == true);
    REQUIRE(value == "test-1234");

    delete data;
}