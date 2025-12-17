#pragma once

#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string_view>

#include <cstddef>
#include <cstdint>

namespace toke {

class Exception : public std::runtime_error
{
public:
  using std::runtime_error::runtime_error;

  ~Exception() override = default;
};

class FileIOError final : public Exception
{
public:
  FileIOError()
    : Exception("file IO error")
  {
  }
};

class FileNotFoundError final : public Exception
{
public:
  FileNotFoundError()
    : Exception("file not found")
  {
  }
};

class MemoryAllocationError final : public Exception
{
public:
  MemoryAllocationError()
    : Exception("failed to allocate memory")
  {
  }
};

class FilterSyntaxError final : public Exception
{
public:
  FilterSyntaxError()
    : Exception("filter syntax error")
  {
  }
};

class VocabSyntaxError final : public Exception
{
public:
  VocabSyntaxError()
    : Exception("vocab syntax error")
  {
  }
};

class TokenArray final
{
public:
  TokenArray() = default;

  TokenArray(TokenArray&& other) noexcept;

  TokenArray(const TokenArray&) = delete;

  ~TokenArray();

  auto operator=(const TokenArray&) = delete;

  auto operator=(TokenArray&&) = delete;

  [[nodiscard]] auto data() const -> const std::uint16_t* { return m_data; }

  [[nodiscard]] auto size() const -> std::size_t { return m_size; }

  [[nodiscard]] auto operator[](const std::size_t index) const -> const std::uint16_t& { return m_data[index]; }

protected:
  friend class Encoder;

  TokenArray(std::uint16_t* data, std::size_t size);

private:
  std::uint16_t* m_data{};

  std::size_t m_size{};
};

class Encoder
{
public:
  [[nodiscard]] static auto create() -> std::unique_ptr<Encoder>;

  virtual ~Encoder() = default;

  virtual void loadVocab(const std::filesystem::path& filename) = 0;

  virtual void parseVocab(const std::string_view& vocab) = 0;

  [[nodiscard]] virtual auto encode(const std::string_view& text) const -> TokenArray = 0;

protected:
  [[nodiscard]] static auto makeTokenArray(std::uint16_t* data, const std::size_t size) -> TokenArray
  {
    return { data, size };
  }
};

class Decoder
{
public:
  [[nodiscard]] static auto create() -> std::unique_ptr<Decoder>;

  virtual ~Decoder() = default;

  virtual void loadVocab(const std::filesystem::path& filename) = 0;

  virtual void parseVocab(const std::string_view& vocab) = 0;

  [[nodiscard]] virtual auto decode(const std::uint16_t* tokens, const std::size_t size) const -> std::string = 0;
};

class Filter
{
public:
  [[nodiscard]] static auto create() -> std::unique_ptr<Filter>;

  virtual ~Filter() = default;

  virtual void parseConfig(const std::string_view& config) = 0;

  [[nodiscard]] virtual auto filter(const std::string_view& input) const -> std::string = 0;
};

} // namespace toke
