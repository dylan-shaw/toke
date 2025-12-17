#pragma once

#include "file_visitor.h"

#include <memory>

class Vocab;

class PairCounter : public FileVisitor
{
public:
  struct Result final
  {
    std::uint16_t left{};

    std::uint16_t right{};

    std::size_t count{};
  };

  [[nodiscard]] static auto create(const Vocab* vocab) -> std::unique_ptr<PairCounter>;

  ~PairCounter() override = default;

  [[nodiscard]] virtual auto getHighestFrequencyPair() const -> Result = 0;
};
