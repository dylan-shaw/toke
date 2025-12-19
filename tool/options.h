#pragma once

#include <filesystem>
#include <string>

struct Options final
{
  std::filesystem::path directoryPath{ "." };

  std::filesystem::path outputPath{ "tokenizer.txt" };

  std::string filterConfig;

  int numTokens{ 1024 };

  bool preload{ true };

  [[nodiscard]] auto parse(int argc, char** argv) -> bool;
};
