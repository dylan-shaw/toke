#pragma once

#include <cstddef>
#include <cstdint>

constexpr size_t
utf8Length(const std::uint8_t lead)
{
  if ((lead >> 5) == 0x06) {
    return 2; // 110xxxxx
  }

  if ((lead >> 4) == 0x0e) {
    return 3; // 1110xxxx
  }

  if ((lead >> 3) == 0x1e) {
    return 4; // 11110xxx
  }

  return 1; // invalid lead byte
}
