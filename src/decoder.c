#include <toke/decoder.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct vocab_entry
{
  char* def;

  size_t size;
};

struct toke_decoder
{
  struct vocab_entry* vocab;

  size_t vocab_size;
};

toke_decoder_z*
toke_decoder_new()
{
  toke_decoder_z* self = (toke_decoder_z*)malloc(sizeof(toke_decoder_z));
  if (!self) {
    return NULL;
  }

  self->vocab = NULL;
  self->vocab_size = 0;

  return self;
}

void
toke_decoder_delete(toke_decoder_z* self)
{
  if (self) {
    for (size_t i = 0; i < self->vocab_size; i++) {
      free(self->vocab[i].def);
    }
    free(self->vocab);
  }

  free(self);
}

static toke_error_z
add_token_def(toke_decoder_z* self, const char* word, const size_t word_len)
{
  const size_t new_size = (self->vocab_size + 1) * sizeof(struct vocab_entry);

  void* ptr = realloc(self->vocab, new_size);
  if (!ptr) {
    return TOKE_ERROR_MEMORY_ALLOCATION;
  }

  self->vocab = (struct vocab_entry*)ptr;

  struct vocab_entry* entry = &self->vocab[self->vocab_size];
  entry->def = malloc(word_len + 1);
  if (!entry->def) {
    return TOKE_ERROR_MEMORY_ALLOCATION;
  }

  if ((word_len == 2) && (word[0] == '\\') && (word[1] == 'n')) {
    entry->def[0] = '\n';
    entry->def[1] = 0;
    entry->def[2] = 0;
    entry->size = 1;
  } else {
    memcpy(entry->def, word, word_len);
    entry->def[word_len] = 0;
    entry->size = word_len;
  }

  self->vocab_size++;

  return TOKE_ERROR_NONE;
}

toke_error_z
toke_decoder_parse_vocab(toke_decoder_z* self, const char* vocab, const size_t length)
{
  size_t offset = 0;

  size_t word_offset = 0;

  while (offset < length) {

    const char c = vocab[offset];

    if (c != '\n') {
      offset++;
      continue;
    }

    const size_t word_len = offset - word_offset;

    const toke_error_z err = add_token_def(self, &vocab[word_offset], word_len);
    if (err != TOKE_ERROR_NONE) {
      return err;
    }

    offset++;

    word_offset = offset;
  }

  if (word_offset < offset) {
    const size_t word_len = offset - word_offset;
    return add_token_def(self, &vocab[word_offset], word_len);
  }

  return TOKE_ERROR_NONE;
}

toke_error_z
toke_decoder_load_vocab(toke_decoder_z* self, const char* filename)
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

  const toke_error_z parse_error = toke_decoder_parse_vocab(self, vocab, read_size);

  free(vocab);

  return parse_error;
}

char*
toke_decode(toke_decoder_z* self, const uint16_t* tokens, const size_t length, size_t* out_length_ptr)
{
  size_t out_length = 0;

  for (size_t i = 0; i < length; i++) {
    const uint8_t token = tokens[i];
    if (token >= self->vocab_size) {
      // for \x7f
      out_length++;
      continue;
    }
    out_length += self->vocab[token].size;
  }

  char* result = malloc(out_length + 1);
  if (!result) {
    return NULL;
  }

  size_t offset = 0;

  for (size_t i = 0; i < length; i++) {
    const uint8_t token = tokens[i];
    if (token >= self->vocab_size) {
      result[offset] = '\x7f';
      offset++;
      continue;
    }
    const struct vocab_entry* entry = &self->vocab[token];
    memcpy(result + offset, entry->def, entry->size);
    offset += entry->size;
  }

  result[out_length] = 0;

  *out_length_ptr = out_length;

  return result;
}
