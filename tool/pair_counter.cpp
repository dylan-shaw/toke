#include "pair_counter.h"

#include <toke/cxx_api.hpp>

#include "vocab.h"

#include <map>
#include <vector>

#include <cstddef>
#include <cstdint>

namespace {

class PairCounterImpl final : public PairCounter
{
public:
  explicit PairCounterImpl(const Vocab* vocab)
    : m_vocab(vocab)
  {
  }

  void prepare(const std::size_t numFiles) override
  {
    m_encoder = toke::Encoder::create();

    m_encoder->parseVocab(m_vocab->toString());

    m_pairs.resize(numFiles);

    for (auto& p : m_pairs) {
      p.clear();
    }
  }

  void visitFile(const std::string_view& data, const std::size_t fileIndex) override
  {
    auto& pairs = m_pairs[fileIndex];

    const auto tokens = m_encoder->encode(data);

    for (size_t i = 1; i < tokens.size(); ++i) {

      const auto l = tokens[i - 1];
      const auto r = tokens[i];

      if ((l == 0xffff) || (r == 0xffff)) {
        // unknown token
        continue;
      }

      if (!isAlpha(m_vocab->at(l)) || !isAlpha(m_vocab->at(r))) {
        continue;
      }

      const auto pair = static_cast<std::size_t>(l) | (static_cast<std::size_t>(r) << 16);

      auto it = pairs.find(pair);
      if (it == pairs.end()) {
        it = pairs.emplace(pair, static_cast<std::size_t>(0)).first;
      }
      it->second++;
    }
  }

  void finalize() override
  {
    m_result.clear();

    for (auto& pairs : m_pairs) {
      for (auto [id, count] : pairs) {
        auto it = m_result.find(id);
        if (it == m_result.end()) {
          it = m_result.emplace(id, static_cast<std::size_t>(0)).first;
        }
        it->second += count;
      }
    }
  }

  [[nodiscard]] auto getHighestFrequencyPair() const -> Result override
  {
    Result result;

    for (auto& pair : m_result) {
      if (pair.second > result.count) {
        result.left = (pair.first >> 0) & 0xffff;
        result.right = (pair.first >> 16) & 0xffff;
        result.count = pair.second;
      }
    }

    return result;
  }

protected:
  [[nodiscard]] static auto isAlpha(const char c) -> bool
  {
    return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'));
  }

  [[nodiscard]] static auto isAlpha(const TokenDef& def) -> bool
  {
    // If the first character is alpha, then they all are.
    // This is because we don't merge tokens that aren't alphabetical.
    return isAlpha(reinterpret_cast<const char*>(def.data())[0]);
  }

private:
  const Vocab* m_vocab;

  std::unique_ptr<toke::Encoder> m_encoder;

  std::vector<std::map<std::uint32_t, std::size_t>> m_pairs;

  std::map<std::uint32_t, std::size_t> m_result;
};

} // namespace

auto
PairCounter::create(const Vocab* vocab) -> std::unique_ptr<PairCounter>
{
  return std::make_unique<PairCounterImpl>(vocab);
}
