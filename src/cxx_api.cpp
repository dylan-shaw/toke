#include <toke/cxx_api.hpp>

#include <toke/decoder.h>
#include <toke/encoder.h>
#include <toke/filter.h>

namespace toke {

TokenArray::TokenArray(std::uint16_t* data, const std::size_t size)
  : m_data(data)
  , m_size(size)
{
}

TokenArray::TokenArray(TokenArray&& other) noexcept
  : m_data(other.m_data)
  , m_size(other.m_size)
{
  other.m_data = nullptr;
  other.m_size = 0;
}

TokenArray::~TokenArray()
{
  std::free(m_data);
}

namespace {

void
checkError(const toke_error_z err)
{
  switch (err) {
    case TOKE_ERROR_NONE:
      break;
    case TOKE_ERROR_FILE_IO:
      throw FileIOError();
    case TOKE_ERROR_FILE_NOT_FOUND:
      throw FileNotFoundError();
    case TOKE_ERROR_MEMORY_ALLOCATION:
      throw MemoryAllocationError();
    case TOKE_ERROR_FILTER_SYNTAX:
      throw FilterSyntaxError();
    case TOKE_ERROR_VOCAB_SYNTAX:
      throw VocabSyntaxError();
  }
}

class EncoderImpl final : public Encoder
{
public:
  EncoderImpl()
    : m_self(toke_encoder_new())
  {
    if (!m_self) {
      throw MemoryAllocationError();
    }
  }

  ~EncoderImpl() override { toke_encoder_delete(m_self); }

  void loadVocab(const std::filesystem::path& filename) override
  {
    const auto err = toke_encoder_load_vocab(m_self, filename.string().c_str());
    checkError(err);
  }

  void parseVocab(const std::string_view& vocab) override
  {
    const auto err = toke_encoder_parse_vocab(m_self, vocab.data(), vocab.size());
    checkError(err);
  }

  [[nodiscard]] auto encode(const std::string_view& text) const -> TokenArray override
  {
    std::size_t length = 0;

    auto* data = toke_encode(m_self, text.data(), text.size(), &length);
    if (!data) {
      throw MemoryAllocationError();
    }

    return makeTokenArray(data, length);
  }

private:
  toke_encoder_z* m_self{};
};

} // namespace

auto
Encoder::create() -> std::unique_ptr<Encoder>
{
  return std::make_unique<EncoderImpl>();
}

namespace {

class DecoderImpl final : public Decoder
{
public:
  DecoderImpl()
    : m_self(toke_decoder_new())
  {
    if (!m_self) {
      throw MemoryAllocationError();
    }
  }

  ~DecoderImpl() override { toke_decoder_delete(m_self); }

  void loadVocab(const std::filesystem::path& filename) override
  {
    const auto err = toke_decoder_load_vocab(m_self, filename.string().c_str());
    checkError(err);
  }

  void parseVocab(const std::string_view& vocab) override
  {
    const auto err = toke_decoder_parse_vocab(m_self, vocab.data(), vocab.size());
    checkError(err);
  }

  [[nodiscard]] auto decode(const std::uint16_t* tokens, const std::size_t numTokens) const -> std::string override
  {
    size_t out_length = 0;

    char* ptr = toke_decode(m_self, tokens, numTokens, &out_length);
    if (!ptr) {
      throw MemoryAllocationError();
    }

    try {
      std::string result(ptr, out_length);
      std::free(ptr);
      return result;
    } catch (...) {
      std::free(ptr);
      throw;
    }
  }

private:
  toke_decoder_z* m_self{};
};

} // namespace

auto
Decoder::create() -> std::unique_ptr<Decoder>
{
  return std::make_unique<DecoderImpl>();
}

namespace {

class FilterImpl final : public Filter
{
public:
  FilterImpl()
    : m_self(toke_filter_new())
  {
    if (!m_self) {
      throw MemoryAllocationError();
    }
  }

  ~FilterImpl() override { toke_filter_delete(m_self); }

  void parseConfig(const std::string_view& config) override
  {
    const auto err = toke_filter_parse_config(m_self, config.data(), config.size());
    checkError(err);
  }

  [[nodiscard]] auto filter(const std::string_view& input) const -> std::string override
  {
    size_t out_length = 0;
    char* result = toke_filter(m_self, input.data(), input.size(), &out_length);
    if (!result) {
      throw MemoryAllocationError();
    }
    try {
      std::string tmp(result, out_length);
      std::free(result);
      return tmp;
    } catch (...) {
      std::free(result);
      throw;
    }
  }

private:
  toke_filter_z* m_self{};
};

} // namespace

auto
Filter::create() -> std::unique_ptr<Filter>
{
  return std::make_unique<FilterImpl>();
}

} // namespace toke
