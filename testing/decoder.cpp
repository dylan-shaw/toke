#include <gtest/gtest.h>

#include <toke/cxx_api.hpp>

#include <cstdint>

namespace {

} // namespace

TEST(Decoder, Decode)
{
  const char vocab[] = "aa\n"
                       "b\n"
                       " \n";

  auto decoder = toke::Decoder::create();
  decoder->parseVocab(vocab);
  auto result = decoder->decode(reinterpret_cast<const std::uint16_t*>("\x01\x00\xff\xff\x02\x00\x00\x00"), 4);
  EXPECT_EQ(result, "b\x7f aa");
}

TEST(Decoder, DecodeWithComplexVocab)
{
  const char vocab[] = "#version:1\n#filter:lowercase=true\n\\0a\na\nb";
  auto decoder = toke::Decoder::create();
  decoder->parseVocab(vocab);
  auto result = decoder->decode(reinterpret_cast<const std::uint16_t*>("\x01\x00\x00\x00\x02\x00"), 3);
  EXPECT_EQ(result, "a\nb");
}
