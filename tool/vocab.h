#pragma once

#include <string>
#include <string_view>
#include <vector>

#include <cstddef>
#include <cstdint>

class TokenDef final
{
public:
  [[nodiscard]] static auto concat(const TokenDef& l, const TokenDef& r) -> TokenDef;

  TokenDef() = default;

  explicit TokenDef(const std::uint8_t c);

  explicit TokenDef(const std::uint8_t* begin, const std::uint8_t* end);

  [[nodiscard]] auto data() const -> const std::uint8_t* { return m_data.data(); }

  [[nodiscard]] auto size() const -> std::size_t { return m_data.size(); }

private:
  std::vector<std::uint8_t> m_data;
};

class Vocab final
{
public:
  explicit Vocab(std::string filterConfig);

  void define(TokenDef def);

  void define(const std::string_view& def);

  [[nodiscard]] auto toString() const -> std::string;

  [[nodiscard]] auto size() const -> std::size_t { return m_defs.size(); }

  [[nodiscard]] auto at(const std::size_t index) const -> const TokenDef& { return m_defs.at(index); }

private:
  std::string m_filterConfig;

  std::vector<TokenDef> m_defs;
};
