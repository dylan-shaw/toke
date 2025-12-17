#include "vocab.h"

#include <iomanip>
#include <sstream>

auto
TokenDef::concat(const TokenDef& l, const TokenDef& r) -> TokenDef
{
  TokenDef result;

  result.m_data.resize(l.m_data.size() + r.m_data.size());

  for (std::size_t i = 0; i < l.m_data.size(); i++) {
    result.m_data[i] = l.m_data[i];
  }

  const auto offset = l.m_data.size();

  for (std::size_t i = 0; i < r.m_data.size(); i++) {
    result.m_data[i + offset] = r.m_data[i];
  }

  return result;
}

TokenDef::TokenDef(const std::uint8_t c)
  : m_data(&c, &c + 1)
{
}

TokenDef::TokenDef(const std::uint8_t* begin, const std::uint8_t* end)
  : m_data(begin, end)
{
}

Vocab::Vocab(std::string filterConfig)
  : m_filterConfig(std::move(filterConfig))
{
}

void
Vocab::define(TokenDef def)
{
  m_defs.emplace_back(std::move(def));
}

void
Vocab::define(const std::string_view& def)
{
  const auto* begin = reinterpret_cast<const std::uint8_t*>(def.data());
  const auto* end = begin + def.size();
  m_defs.emplace_back(begin, end);
}

namespace {

[[nodiscard]] auto
escape(const std::uint8_t* def, const std::size_t defSize) -> std::string
{
  std::ostringstream what;

  for (std::size_t i = 0; i < defSize; i++) {

    const auto value = def[i];

    if ((value >= 33) && (value <= 126) && (value != '\\') && (value != '#')) {
      what << *reinterpret_cast<const char*>(def + i);
      continue;
    }

    what << '\\' << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(value);
  }

  return what.str();
}

} // namespace

auto
Vocab::toString() const -> std::string
{
  std::ostringstream vocabStream;
  vocabStream << "#version:1\n";
  vocabStream << "#filter:" << m_filterConfig << '\n';
  for (const auto& tokenDef : m_defs) {
    vocabStream << escape(tokenDef.data(), tokenDef.size()) << '\n';
  }
  return vocabStream.str();
}
