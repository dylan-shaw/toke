#pragma once

#include <string_view>

#include <cstddef>

class FileVisitor
{
public:
  virtual ~FileVisitor() = default;

  virtual void prepare(std::size_t numFiles) = 0;

  virtual void visitFile(const std::string_view& data, const std::size_t index) = 0;

  virtual void finalize() = 0;
};
