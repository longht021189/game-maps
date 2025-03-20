#ifndef GAME_MAPS_CORE_DATA_HPP
#define GAME_MAPS_CORE_DATA_HPP

#include <string>

struct Data {
  static Data* get_instance();

  virtual ~Data() = default;

  virtual bool write(std::string key, std::string value) = 0;
  virtual bool read(std::string key, std::string &value) = 0;

private:
  static Data* instance;
};

#endif // GAME_MAPS_CORE_DATA_HPP