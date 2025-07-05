#ifndef UTILS_H
#define UTILS_H
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

namespace quarry {
void load_dotenv(const std::string &path = ".env") {
  std::ifstream env_file(path);
  if (!env_file.is_open()) {
    std::cerr << "Could not open .env file: " << path << std::endl;
    return;
  }

  std::string line;
  while (std::getline(env_file, line)) {
    if (line.empty() || line[0] == '#')
      continue;

    std::istringstream is_line(line);
    std::string key;
    if (std::getline(is_line, key, '=')) {
      std::string value;
      if (std::getline(is_line, value)) {
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t\""));
        value.erase(value.find_last_not_of(" \t\"") + 1);

        setenv(key.c_str(), value.c_str(), 1); // overwrite = true
      }
    }
  }
}
} // namespace quarry
#endif
