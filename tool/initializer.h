#pragma once

#include "file_visitor.h"

#include <memory>

class Vocab;

class Initializer : public FileVisitor
{
public:
  [[nodiscard]] static auto create(Vocab* vocab) -> std::unique_ptr<Initializer>;

  ~Initializer() override = default;
};
