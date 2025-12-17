#include <toke/filter.h>

#include <toke/error.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum flag
{
  NORMALIZE_NEWLINES = 0x01,
  NORMALIZE_TABS = 0x02,
  RESTRICTED_ASCII = 0x04,
  LOWERCASE = 0x08,
  UNICODE_SUBSTITUTES = 0x10
};

struct config
{
  int flags;
};

static struct config
default_config()
{
  struct config cfg;
  cfg.flags = 0;
  return cfg;
}

struct toke_filter
{
  struct config config;
};

toke_filter_z*
toke_filter_new()
{
  toke_filter_z* self = malloc(sizeof(toke_filter_z));
  if (!self) {
    return NULL;
  }

  self->config = default_config();

  return self;
}

void
toke_filter_delete(toke_filter_z* self)
{
  free(self);
}

static size_t
find_char(const char* config, const size_t length, const size_t offset, const char c)
{
  for (size_t i = offset; i < length; i++) {
    if (config[i] == c) {
      return i;
    }
  }
  return length;
}

toke_error_z
toke_filter_parse_config(toke_filter_z* self, const char* config, const size_t length)
{
  self->config = default_config();

  size_t prop_start = 0;

  while (prop_start < length) {

    const size_t prop_end = find_char(config, length, prop_start, ',');
    const size_t key_end = find_char(config, length, prop_start, '=');
    if (prop_end < key_end) {
      return TOKE_ERROR_FILTER_SYNTAX;
    }

    const char* key = config + prop_start;
    const size_t key_len = key_end - prop_start;

    const char* value = config + key_end + 1;
    const size_t value_len = prop_end - (key_end + 1);

#define MATCH_KEY(other_key) ((sizeof(other_key) - 1) == key_len) && (memcmp(other_key, key, key_len) == 0)

#define MATCH_VALUE(other_value)                                                                                       \
  ((sizeof(other_value) - 1) == value_len) && (memcmp(other_value, value, value_len) == 0)

    if (MATCH_KEY("normalize_lines")) {
      if (MATCH_VALUE("true")) {
        self->config.flags |= NORMALIZE_NEWLINES;
      } else if (MATCH_VALUE("false")) {
        self->config.flags &= ~NORMALIZE_NEWLINES;
      } else {
        return TOKE_ERROR_FILTER_SYNTAX;
      }
    }

    if (MATCH_KEY("normalize_tabs")) {
      if (MATCH_VALUE("true")) {
        self->config.flags |= NORMALIZE_TABS;
      } else if (MATCH_VALUE("false")) {
        self->config.flags &= ~NORMALIZE_TABS;
      } else {
        return TOKE_ERROR_FILTER_SYNTAX;
      }
    }

    if (MATCH_KEY("restricted_ascii")) {
      if (MATCH_VALUE("true")) {
        self->config.flags |= RESTRICTED_ASCII;
      } else if (MATCH_VALUE("false")) {
        self->config.flags &= ~RESTRICTED_ASCII;
      } else {
        return TOKE_ERROR_FILTER_SYNTAX;
      }
    }

    if (MATCH_KEY("lowercase")) {
      if (MATCH_VALUE("true")) {
        self->config.flags |= LOWERCASE;
      } else if (MATCH_VALUE("false")) {
        self->config.flags &= ~LOWERCASE;
      } else {
        return TOKE_ERROR_FILTER_SYNTAX;
      }
    }

    if (MATCH_KEY("unicode_substitutes")) {
      if (MATCH_VALUE("true")) {
        self->config.flags |= UNICODE_SUBSTITUTES;
      } else if (MATCH_VALUE("false")) {
        self->config.flags &= ~UNICODE_SUBSTITUTES;
      } else {
        return TOKE_ERROR_FILTER_SYNTAX;
      }
    }

    prop_start = prop_end + 1;
  }

  return TOKE_ERROR_NONE;
}

static uint8_t
is_restricted_ascii(const char c)
{
  uint8_t value = 0;
  value |= (c >= ' ') && (c <= '~');
  value |= (c == '\r');
  value |= (c == '\n');
  value |= (c == '\t');
  return value;
}

static size_t
utf8_length(const char c)
{
  const uint8_t lead = *(const uint8_t*)&c;

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

char*
toke_filter(toke_filter_z* filter, const char* input, const size_t length, size_t* out_length_ptr)
{
  char* result = malloc(length + 1);
  if (!result) {
    return NULL;
  }

  size_t src_offset = 0;
  size_t dst_offset = 0;

  const uint8_t normalize_newlines = filter->config.flags & NORMALIZE_NEWLINES;
  const uint8_t normalize_tabs = filter->config.flags & NORMALIZE_TABS;
  const uint8_t restricted_ascii = filter->config.flags & RESTRICTED_ASCII;
  const uint8_t lowercase = filter->config.flags & LOWERCASE;
  const uint8_t unicode_subs = filter->config.flags & UNICODE_SUBSTITUTES;

  while (src_offset < length) {

    const size_t remaining = length - src_offset;
    const char c = input[src_offset];
    const size_t code_len = utf8_length(c);

    if (normalize_newlines && (c == '\r')) {
      if ((remaining > 1) && (input[src_offset + 1] == '\n')) {
        result[dst_offset] = '\n';
        dst_offset++;
        src_offset += 2;
        continue;
      } else {
        result[dst_offset] = '\n';
        dst_offset++;
        src_offset++;
        continue;
      }
    }

    if (normalize_tabs && (c == '\t')) {
      result[dst_offset] = ' ';
      dst_offset++;
      src_offset++;
      continue;
    }

    if (unicode_subs && (code_len > 1)) {

      if (code_len == 3) {

        // em dash
        if (memcmp(input + src_offset, "\xe2\x80\x94", 3) == 0) {
          result[dst_offset] = '-';
          dst_offset++;
          src_offset += code_len;
          continue;
        }

        // en dash
        if (memcmp(input + src_offset, "\xe2\x80\x93", 3) == 0) {
          result[dst_offset] = '-';
          dst_offset++;
          src_offset += code_len;
          continue;
        }

        // hyphen
        if (memcmp(input + src_offset, "\xe2\x80\x90", 3) == 0) {
          result[dst_offset] = '-';
          dst_offset++;
          src_offset += code_len;
          continue;
        }

        // hyphen (non-breaking)
        if (memcmp(input + src_offset, "\xe2\x80\x91", 3) == 0) {
          result[dst_offset] = '-';
          dst_offset++;
          src_offset += code_len;
          continue;
        }

        // double quote
        if (memcmp(input + src_offset, "\xe2\x80\x9c", 3) == 0) {
          result[dst_offset] = '"';
          dst_offset++;
          src_offset += code_len;
          continue;
        }

        // double quote
        if (memcmp(input + src_offset, "\xe2\x80\x9d", 3) == 0) {
          result[dst_offset] = '"';
          dst_offset++;
          src_offset += code_len;
          continue;
        }

        // double quote
        if (memcmp(input + src_offset, "\xe2\x80\xb3", 3) == 0) {
          result[dst_offset] = '"';
          dst_offset++;
          src_offset += code_len;
          continue;
        }

        // single quote
        if (memcmp(input + src_offset, "\xe2\x80\x98", 3) == 0) {
          result[dst_offset] = '\'';
          dst_offset++;
          src_offset += code_len;
          continue;
        }

        // single quote
        if (memcmp(input + src_offset, "\xe2\x80\x99", 3) == 0) {
          result[dst_offset] = '\'';
          dst_offset++;
          src_offset += code_len;
          continue;
        }

        // single quote
        if (memcmp(input + src_offset, "\xe2\x80\xb2", 3) == 0) {
          result[dst_offset] = '\'';
          dst_offset++;
          src_offset += code_len;
          continue;
        }

        // bullet
        if (memcmp(input + src_offset, "\xe2\x80\xa2", 3) == 0) {
          result[dst_offset] = '*';
          dst_offset++;
          src_offset += code_len;
          continue;
        }

        // bullet
        if (memcmp(input + src_offset, "\xe2\x80\xa3", 3) == 0) {
          result[dst_offset] = '*';
          dst_offset++;
          src_offset += code_len;
          continue;
        }
      }
    }

    if (restricted_ascii && !is_restricted_ascii(c)) {
      result[dst_offset] = '\x7f';
      dst_offset++;
      src_offset += code_len;
      continue;
    }

    if (lowercase && ((c >= 'A') && (c <= 'Z'))) {
      result[dst_offset] = (char)(c + 32);
      dst_offset++;
      src_offset++;
      continue;
    }

    // normal condition
    for (size_t i = 0; i < code_len; i++) {
      result[dst_offset + i] = input[src_offset + i];
    }
    dst_offset += code_len;
    src_offset += code_len;
  }

  result[dst_offset] = 0;

  if (out_length_ptr) {
    *out_length_ptr = dst_offset;
  }

  return result;
}
