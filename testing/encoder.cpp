#include <gtest/gtest.h>

#include <toke/cxx_api.hpp>

namespace {

constexpr char vocabPlain[] = R"(a
b
c
aa
bb
 
\n
)";

constexpr char vocabWithDirectives[] = R"(#version:1
#filter:lowercase=true
a
b
aa
)";

constexpr char vocabWithEmptyFilter[] = R"(#version:1
#filter:
a
b
)";

} // namespace

TEST(Encoder, Encode)
{
  const auto encoder = toke::Encoder::create();
  encoder->parseVocab(vocabPlain);
  const auto tokens = encoder->encode("a bb aacd");
  ASSERT_EQ(tokens.size(), 7);
  EXPECT_EQ(tokens[0], 0);
  EXPECT_EQ(tokens[1], 5);
  EXPECT_EQ(tokens[2], 4);
  EXPECT_EQ(tokens[3], 5);
  EXPECT_EQ(tokens[4], 3);
  EXPECT_EQ(tokens[5], 2);
  EXPECT_EQ(tokens[6], 65535);
}

TEST(Encoder, ParseComplexVocab)
{
  const auto encoder = toke::Encoder::create();
  encoder->parseVocab(vocabWithDirectives);
  const auto tokens = encoder->encode("Aab");
  ASSERT_EQ(tokens.size(), 2);
  EXPECT_EQ(tokens[0], 2);
  EXPECT_EQ(tokens[1], 1);
}

TEST(Encoder, ParseVocabWithEmptyFilter)
{
  const auto encoder = toke::Encoder::create();
  encoder->parseVocab(vocabWithEmptyFilter);
  const auto tokens = encoder->encode("ba");
  ASSERT_EQ(tokens.size(), 2);
  EXPECT_EQ(tokens[0], 1);
  EXPECT_EQ(tokens[1], 0);
}