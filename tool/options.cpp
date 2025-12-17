#include "options.h"

#include <iostream>

#include <cstring>

[[nodiscard]] auto
Options::parse(int argc, char** argv) -> bool
{
  for (int i = 1; i < argc; i++) {
    if (std::strcmp(argv[i], "--filter") == 0) {
      if ((i + 1) >= argc) {
        std::cerr << "missing value for " << argv[i] << std::endl;
        return false;
      }
      this->filterConfig = argv[i + 1];
      i++;
    } else if (std::strcmp(argv[i], "--directory") == 0) {
      if ((i + 1) >= argc) {
        std::cerr << "missing value for " << argv[i] << std::endl;
        return false;
      }
      this->directoryPath = argv[i + 1];
      i++;
    } else if (std::strcmp(argv[i], "--output") == 0) {
      if ((i + 1) >= argc) {
        std::cerr << "missing value for " << argv[i] << std::endl;
        return false;
      }
      this->outputPath = argv[i + 1];
      i++;
    } else if (std::strcmp(argv[i], "--no-preload") == 0) {
      this->preload = false;
    } else if (std::strcmp(argv[i], "--preload") == 0) {
      this->preload = true;
    } else {
      std::cerr << "unknown option: " << argv[i] << std::endl;
      return false;
    }
  }

  return true;
}
