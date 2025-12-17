#include <gtest/gtest.h>

#include <toke/cxx_api.hpp>

TEST(Filter, NormalizeLines)
{
  {
    const auto filter = toke::Filter::create();
    filter->parseConfig("normalize_lines=false");
    const auto output = filter->filter("a\r\nb\nc\rd");
    EXPECT_EQ(output, "a\r\nb\nc\rd");
  }

  {
    const auto filter = toke::Filter::create();
    filter->parseConfig("normalize_lines=true");
    const auto output = filter->filter("a\r\nb\nc\rd");
    EXPECT_EQ(output, "a\nb\nc\nd");
  }
}

TEST(Filter, RestrictedAscii)
{
  constexpr char testVector[] = "\x01 \x7f\n\x1d";

  {
    const auto filter = toke::Filter::create();
    filter->parseConfig("restricted_ascii=false");
    const auto output = filter->filter(testVector);
    EXPECT_EQ(output, testVector);
  }

  {
    const auto filter = toke::Filter::create();
    filter->parseConfig("restricted_ascii=true");
    const auto output = filter->filter(testVector);
    EXPECT_EQ(output, "\x7f \x7f\n\x7f");
  }
}

TEST(Filter, Lowercase)
{
  constexpr char testVector[] = "aBc";

  {
    const auto filter = toke::Filter::create();
    filter->parseConfig("lowercase=false");
    const auto output = filter->filter(testVector);
    EXPECT_EQ(output, testVector);
  }

  {
    const auto filter = toke::Filter::create();
    filter->parseConfig("lowercase=true");
    const auto output = filter->filter(testVector);
    EXPECT_EQ(output, "abc");
  }
}

TEST(Filter, NormalizeTabs)
{
  constexpr char testVector[] = " \t ";

  {
    const auto filter = toke::Filter::create();
    filter->parseConfig("normalize_tabs=false");
    const auto output = filter->filter(testVector);
    EXPECT_EQ(output, testVector);
  }

  {
    const auto filter = toke::Filter::create();
    filter->parseConfig("normalize_tabs=true");
    const auto output = filter->filter(testVector);
    EXPECT_EQ(output, "   ");
  }
}

TEST(Filter, Compound)
{
  constexpr char testVector[] = "\t\r\n";

  {
    const auto filter = toke::Filter::create();
    filter->parseConfig("");
    const auto output = filter->filter(testVector);
    EXPECT_EQ(output, testVector);
  }

  {
    const auto filter = toke::Filter::create();
    filter->parseConfig("normalize_tabs=true,normalize_lines=true");
    const auto output = filter->filter(testVector);
    EXPECT_EQ(output, " \n");
  }
}

TEST(Filter, UnicodeSubstitutions)
{
  {
    // em dash, en dash
    constexpr char testVector[] = "\xe2\x80\x94 \xe2\x80\x93";
    const auto filter = toke::Filter::create();
    filter->parseConfig("");
    const auto output = filter->filter(testVector);
    EXPECT_EQ(output, testVector);
  }

  {
    // em dash, en dash, hyphen, non-breaking hyphen
    constexpr char testVector[] = "\xe2\x80\x94 \xe2\x80\x93 \xe2\x80\x90 \xe2\x80\x91";
    const auto filter = toke::Filter::create();
    filter->parseConfig("unicode_substitutes=true");
    const auto output = filter->filter(testVector);
    EXPECT_EQ(output, "- - - -");
  }

  {
    // single quotes
    constexpr char testVector[] = "\xe2\x80\x98 \xe2\x80\x99 \xe2\x80\xb2";
    const auto filter = toke::Filter::create();
    filter->parseConfig("unicode_substitutes=true");
    const auto output = filter->filter(testVector);
    EXPECT_EQ(output, "' ' '");
  }

  {
    // double quotes
    constexpr char testVector[] = "\xe2\x80\x9c \xe2\x80\x9d \xe2\x80\xb3";
    const auto filter = toke::Filter::create();
    filter->parseConfig("unicode_substitutes=true");
    const auto output = filter->filter(testVector);
    EXPECT_EQ(output, "\" \" \"");
  }

  {
    // bullets
    constexpr char testVector[] = "\xe2\x80\xa2 \xe2\x80\xa3";
    const auto filter = toke::Filter::create();
    filter->parseConfig("unicode_substitutes=true");
    const auto output = filter->filter(testVector);
    EXPECT_EQ(output, "* *");
  }
}
