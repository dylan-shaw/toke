#include "initializer.h"

#include "utf8.h"
#include "vocab.h"

#include <set>
#include <vector>

namespace {

class InitializerImpl final : public Initializer
{
public:
  explicit InitializerImpl(Vocab* vocab)
    : m_vocab(vocab)
  {
  }

  void prepare(const std::size_t numFiles) override
  {
    m_defs.resize(numFiles);

    for (auto& defs : m_defs) {
      defs.clear();
    }
  }

  void visitFile(const std::string_view& data, const std::size_t fileIndex) override
  {
    auto& defs = m_defs.at(fileIndex);

    std::size_t offset = 0;

    const auto* ptr = reinterpret_cast<const std::uint8_t*>(data.data());

    while (offset < data.size()) {

      const uint8_t lead = ptr[offset];

      const auto remaining = data.size() - offset;

      const auto size = utf8Length(lead);

      if (size > remaining) {
        // this is probably the last code point in the file, but it's incomplete,
        // so let's not even bother because we can't reconstruct it anyway
        break;
      }

      defs.emplace(std::string(reinterpret_cast<const char*>(ptr + offset), size));

      offset += size;
    }
  }

  void finalize() override
  {
    std::set<std::string> combined;

    for (const auto& defs : m_defs) {
      for (auto& def : defs) {
        combined.emplace(def);
      }
    }

    for (auto& def : combined) {
      m_vocab->define(def);
    }
  }

private:
  Vocab* m_vocab{};

  std::vector<std::set<std::string>> m_defs;
};

} // namespace

auto
Initializer::create(Vocab* vocab) -> std::unique_ptr<Initializer>
{
  return std::make_unique<InitializerImpl>(vocab);
}
