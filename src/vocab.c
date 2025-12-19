#include "vocab.h"

#include <stdlib.h>

static uint8_t
hex_to_value(const char value)
{
  if ((value >= 'a') && (value <= 'f')) {
    return 10 + (value - 'a');
  }

  if ((value >= '0') && (value <= '9')) {
    return value - '0';
  }

  return 0;
}

uint8_t*
toke_process_token_def(const char* word, const size_t length, size_t* out_size)
{
  uint8_t* result = malloc(length + 1);
  if (!result) {
    return NULL;
  }

  size_t src_offset = 0;
  size_t dst_offset = 0;

  while (src_offset < length) {

    if (word[src_offset] != '\\') {
      result[dst_offset] = *(const uint8_t*)(word + src_offset);
      dst_offset++;
      src_offset++;
      continue;
    }

    src_offset++;

    uint8_t value = 0;

    const size_t hex_start = src_offset;

    while (src_offset < length) {

      value <<= 4;

      value |= hex_to_value(word[src_offset]);

      src_offset++;

      if ((src_offset - hex_start) == 2) {
        break;
      }
    }

    result[dst_offset] = value;

    dst_offset++;
  }

  *out_size = dst_offset;

  return result;
}
