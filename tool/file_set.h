#pragma once

#include <filesystem>
#include <memory>
#include <string_view>

class FileVisitor;

class FileSet
{
public:
  [[nodiscard]] static auto create(const std::filesystem::path& root, bool cache, const std::string_view& filterConfig)
    -> std::unique_ptr<FileSet>;

  virtual ~FileSet() = default;

  virtual void accept(FileVisitor& visitor) const = 0;
};
