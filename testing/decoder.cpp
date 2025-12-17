#include <gtest/gtest.h>

#include <toke/cxx_api.hpp>

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
