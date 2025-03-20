#include "utils.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Basic replacement test", "[game][maps][core][utils][1]") {
  std::string str = "Hello world";
  replace(str, "world", "universe");
  REQUIRE(str == "Hello universe");
}

TEST_CASE("Multiple occurrences", "[game][maps][core][utils][2]") {
  std::string str = "Hello hello HELLO";
  replace(str, "hello", "hi");
  // REQUIRE(str == "Hi hi HELLO"); // Case-sensitive
  REQUIRE(str == "Hello hi HELLO");
}

TEST_CASE("No occurrence", "[game][maps][core][utils][3]") {
  std::string str = "Hello world";
  replace(str, "xyz", "abc");
  REQUIRE(str == "Hello world");
}

TEST_CASE("Empty string to replace", "[game][maps][core][utils][4]") {
  std::string str = "Hello world";
  replace(str, "", "abc");
  REQUIRE(str == "Hello world");
}

TEST_CASE("Empty input string", "[game][maps][core][utils][5]") {
  std::string str;
  replace(str, "hello", "hi");
  REQUIRE(str.empty());
}

TEST_CASE("Replace with empty string", "[game][maps][core][utils][6]") {
  std::string str = "Hello world";
  replace(str, "world", "");
  REQUIRE(str == "Hello ");
}

TEST_CASE("Overlapping matches", "[game][maps][core][utils][7]") {
  std::string str = "aaaaa";
  replace(str, "aa", "b");
  REQUIRE(str == "bba"); // Should replace non-overlapping instances
}

TEST_CASE("Replace longer with shorter", "[game][maps][core][utils][8]") {
  std::string str = "This is a long string";
  replace(str, "long string", "short");
  REQUIRE(str == "This is a short");
}

TEST_CASE("Replace shorter with longer", "[game][maps][core][utils][9]") {
  std::string str = "Short text";
  replace(str, "text", "paragraph");
  REQUIRE(str == "Short paragraph");
}

TEST_CASE("Special characters", "[game][maps][core][utils][10]") {
  std::string str = "Hello\nworld\t!";
  replace(str, "\n", " ");
  REQUIRE(str == "Hello world\t!");
}

TEST_CASE("Replace within a specific range", "[game][maps][core][utils][11]") {
  std::string str = "hello world hello universe hello galaxy";
  constexpr size_t start = 6;  // After "hello " (world starts here)
  constexpr size_t end = 22;   // Before " hello galaxy" (covers "world hello universe")

  replace_range(str, "hello", "hi", start, end);

  // Only the "hello" between world and universe (positions 12-16) should be replaced
  REQUIRE(str == "hello world hi universe hello galaxy");
}

TEST_CASE("Multiple replacements within range with length change", "[game][maps][core][utils][12]") {
  std::string str = "aa aa aa aa";
  constexpr size_t start = 3;  // After first "aa "
  constexpr size_t end = 9;    // Before last " aa"

  replace_range(str, "aa", "b", start, end);

  // Replaces "aa" at positions 3-4 and 6-7 with "b", adjusting for length change
  REQUIRE(str == "aa b b aa");
}
