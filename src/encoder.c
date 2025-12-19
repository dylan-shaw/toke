#include <toke/encoder.h>

#include <toke/filter.h>

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vocab.h"

#define INVALID_TOKEN_ID 65535

/**
 * @brief The maximum number of tokens we can define.
 * */
#define MAX_TOKEN_ID 65535

struct trie_node
{
  struct trie_node* nodes[256];

  uint16_t token_id;
};

static void
free_trie(struct trie_node* parent)
{
  for (size_t i = 0; i < 256; i++) {
    if (parent->nodes[i]) {
      free_trie(parent->nodes[i]);
      free(parent->nodes[i]);
    }
  }
}

struct toke_encoder
{
  struct trie_node root;

  /**
   * @brief The token ID to use for unknown tokens.
   * */
  uint32_t unknown_token_id;

  /**
   * @brief An optional text filter.
   * */
  toke_filter_z* filter;
};

toke_encoder_z*
toke_encoder_new()
{
  toke_encoder_z* self = (toke_encoder_z*)malloc(sizeof(toke_encoder_z));
  if (!self) {
    return NULL;
  }

  self->unknown_token_id = 0;
  self->filter = NULL;

  for (size_t i = 0; i < 256; i++) {
    self->root.nodes[i] = NULL;
  }

  self->root.token_id = INVALID_TOKEN_ID;

  return self;
}

void
toke_encoder_delete(toke_encoder_z* self)
{
  if (self) {

    free_trie(&self->root);

    if (self->filter) {
      toke_filter_delete(self->filter);
    }
  }

  free(self);
}

toke_error_z
toke_encoder_load_vocab(toke_encoder_z* self, const char* filename)
{
  FILE* file = fopen(filename, "rb");
  if (!file) {
    return TOKE_ERROR_FILE_NOT_FOUND;
  }

  fseek(file, 0, SEEK_END);

  const long int file_size = ftell(file);
  if (file_size < 0L) {
    fclose(file);
    return TOKE_ERROR_FILE_IO;
  }

  fseek(file, 0, SEEK_SET);

  char* vocab = (char*)malloc(file_size + 1);
  if (!vocab) {
    fclose(file);
    return TOKE_ERROR_MEMORY_ALLOCATION;
  }

  const size_t read_size = fread(vocab, 1, file_size, file);

  fclose(file);

  if (read_size != ((size_t)file_size)) {
    free(vocab);
    return TOKE_ERROR_FILE_IO;
  }

  vocab[read_size] = 0;

  const toke_error_z parse_error = toke_encoder_parse_vocab(self, vocab, read_size);

  free(vocab);

  return parse_error;
}

static size_t
line_length(const char* s, const size_t size, const size_t offset)
{
  for (size_t i = offset; i < size; i++) {
    if (s[i] == '\n') {
      return i - offset;
    }
  }
  return size - offset;
}

static toke_error_z
add_token_def(toke_encoder_z* self, const unsigned char* word, const size_t length, const uint32_t token_id)
{
  if (length == 0) {
    return TOKE_ERROR_NONE;
  }

  struct trie_node* parent = &self->root;

  for (size_t i = 0; i < length; i++) {
    const unsigned char c = word[i];
    if (!parent->nodes[c]) {
      parent->nodes[c] = (struct trie_node*)malloc(sizeof(struct trie_node));
      if (!parent->nodes[c]) {
        return TOKE_ERROR_MEMORY_ALLOCATION;
      }
      struct trie_node* tmp = parent->nodes[c];
      tmp->token_id = INVALID_TOKEN_ID;
      for (size_t i = 0; i < 256; i++) {
        tmp->nodes[i] = NULL;
      }
    }
    parent = parent->nodes[c];
  }

  parent->token_id = token_id;

  return TOKE_ERROR_NONE;
}

static size_t
find_line_size(const char* vocab, const size_t length, const size_t offset)
{
  for (size_t i = offset; i < length; i++) {
    if (vocab[i] == '\n') {
      return i - offset;
    }
  }

  return length - offset;
}

static toke_error_z
parse_directive(toke_encoder_z* self, const char* vocab, const size_t length, const size_t offset, size_t* out_size_ptr)
{
  const size_t size = find_line_size(vocab, length, offset);

  *out_size_ptr = size + 1;

  size_t directive_len = 0;

  while (directive_len < size) {
    const char c = vocab[offset + directive_len];
    if (c == ':') {
      break;
    }
    directive_len++;
  }

  const char* directive = vocab + offset;

  const char* value = vocab + offset + directive_len + 1;

  const size_t value_len = size - (directive_len + 1);

#define MATCH_DIRECTIVE(keyword)                                                                                       \
  (directive_len == (sizeof(keyword) - 1)) && (memcmp(keyword, directive, directive_len) == 0)

  if (MATCH_DIRECTIVE("version")) {
  } else if (MATCH_DIRECTIVE("filter")) {
    if (self->filter) {
      toke_filter_delete(self->filter);
    }
    self->filter = toke_filter_new();
    return toke_filter_parse_config(self->filter, value, value_len);
  } else {
    return TOKE_ERROR_VOCAB_SYNTAX;
  }

  return TOKE_ERROR_NONE;
}

toke_error_z
toke_encoder_parse_vocab(toke_encoder_z* self, const char* vocab, const size_t length)
{
  size_t offset = 0;

  uint32_t token_id = 0;

  while (offset < length) {

    if (vocab[offset] == '#') {
      size_t skip = 0;
      const toke_error_z err = parse_directive(self, vocab, length, offset + 1, &skip);
      if (err != TOKE_ERROR_NONE) {
        return err;
      }
      offset += skip + 1;
      continue;
    }

    const size_t word_size = line_length(vocab, length, offset);

    const char* word = vocab + offset;

    size_t def_size = 0;

    uint8_t* def = toke_process_token_def(word, word_size, &def_size);
    if (!def) {
      return TOKE_ERROR_MEMORY_ALLOCATION;
    }

    const toke_error_z err = add_token_def(self, def, def_size, token_id);

    free(def);

    if (err != TOKE_ERROR_NONE) {
      return err;
    }

    token_id++;

    if (token_id == (MAX_TOKEN_ID - 1)) {
      // max token definitions reached
      // we leave one for "unknown" token IDs
      break;
    }

    offset += word_size + 1;
  }

  self->unknown_token_id = token_id;

  return TOKE_ERROR_NONE;
}

static uint32_t
tokenize_once(const toke_encoder_z* self,
              const uint8_t* ptr,
              const size_t length,
              const size_t offset,
              size_t* word_size_ptr)
{
  size_t word_size = 0;
  const struct trie_node* parent = &self->root;

  const struct trie_node* best = parent;
  size_t best_word_size = 0;

  while ((offset + word_size) < length) {
    const uint8_t c = ptr[offset + word_size];
    if (!parent->nodes[c]) {
      break;
    }
    parent = parent->nodes[c];
    word_size++;
    if (parent->token_id != INVALID_TOKEN_ID) {
      best = parent;
      best_word_size = word_size;
    }
  }

  if (best->token_id == INVALID_TOKEN_ID) {
    // fail safe
    *word_size_ptr = 1;
    return INVALID_TOKEN_ID;
  }

  *word_size_ptr = best_word_size;

  return best->token_id;
}

static uint16_t*
encode(toke_encoder_z* self, const void* text, const size_t length, size_t* out_length)
{
  const uint8_t* ptr = (const uint8_t*)text;

  size_t offset = 0;

  // We allocate once, the size of the input, because the output will not be larger than the input.
  // It's a bit wasteful, but it is very fast
  uint16_t* output = malloc(length * sizeof(uint16_t));
  if (!output) {
    return NULL;
  }

  size_t output_size = 0;

  while (offset < length) {

    size_t word_size = 0;

    const uint32_t token_id = tokenize_once(self, ptr, length, offset, &word_size);

    output[output_size] = (uint16_t)token_id;

    output_size++;

    offset += word_size;
  }

  *out_length = output_size;

  return output;
}

uint16_t*
toke_encode(toke_encoder_z* self, const void* text, const size_t length, size_t* out_length)
{
  if (self->filter) {

    size_t filtered_len = 0;

    char* filtered = toke_filter(self->filter, (const char*)text, length, &filtered_len);
    if (!filtered) {
      return NULL;
    }

    uint16_t* result = encode(self, filtered, filtered_len, out_length);

    free(filtered);

    return result;
  }

  return encode(self, text, length, out_length);
}
