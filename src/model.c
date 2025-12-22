#include <toke/model.h>

#include <stdlib.h>
#include <string.h>

struct token_def
{
  uint8_t* data;

  size_t size;
};

struct toke_model
{
  struct token_def* defs;

  size_t num_defs;
};

toke_model_z*
toke_model_new()
{
  toke_model_z* self = malloc(sizeof(toke_model_z));
  if (!self) {
    return NULL;
  }

  self->defs = NULL;
  self->num_defs = 0;

  return self;
}

void
toke_model_delete(toke_model_z* self)
{
  if (self) {
    for (size_t i = 0; i < self->num_defs; i++) {
      free(self->defs[i].data);
    }
    free(self->defs);
  }

  free(self);
}

size_t
toke_model_size(const toke_model_z* self)
{
  return self->num_defs;
}

struct token_def_key
{
  const uint8_t* data;

  size_t size;
};

static int
cmp_token_defs_const(const void* l, const void* r)
{
  const struct token_def* l_tok = (const struct token_def*)l;
  const struct token_def_key* key = (const struct token_def_key*)r;

  const size_t min_size = l_tok->size < key->size ? l_tok->size : key->size;

  const int cmp = memcmp(l_tok->data, key->data, min_size);
  if (cmp != 0) {
    return cmp;
  }

  if (l_tok->size == key->size) {
    return 0;
  }

  return (l_tok->size < key->size) ? -1 : 1;
}

static const struct token_def*
find_token(const toke_model_z* self, const uint8_t* data, const size_t size)
{
  struct token_def_key key = { .data = data, .size = size };

  return (const struct token_def*)bsearch(
    &key, self->defs, self->num_defs, sizeof(struct token_def), cmp_token_defs_const);
}

static int
cmp_token_defs(const void* l, const void* r)
{
  const struct token_def* l_tok = (const struct token_def*)l;
  const struct token_def* r_tok = (const struct token_def*)r;

  const size_t min_size = l_tok->size < r_tok->size ? l_tok->size : r_tok->size;

  const int cmp = memcmp(l_tok->data, r_tok->data, min_size);
  if (cmp != 0) {
    return cmp;
  }

  if (l_tok->size == r_tok->size) {
    return 0;
  }

  return (l_tok->size < r_tok->size) ? -1 : 1;
}

toke_error_z
toke_model_add_token(toke_model_z* self, const uint8_t* data, const size_t size)
{
  if (find_token(self, data, size) != NULL) {
    return TOKE_ERROR_NONE;
  }

  struct token_def* defs = realloc(self->defs, (self->num_defs + 1) * sizeof(struct token_def));
  if (!defs) {
    return TOKE_ERROR_MEMORY_ALLOCATION;
  }

  self->defs = defs;

  uint8_t* tmp = malloc(size + 1);
  if (!tmp) {
    return TOKE_ERROR_MEMORY_ALLOCATION;
  }

  memcpy(tmp, data, size);

  tmp[size] = 0;

  struct token_def* def = &self->defs[self->num_defs];

  def->data = tmp;
  def->size = size;

  self->num_defs++;

  qsort(self->defs, self->num_defs, sizeof(struct token_def), cmp_token_defs);

  return TOKE_ERROR_NONE;
}

static size_t
to_utf8(uint32_t codepoint, uint8_t* utf8)
{
  if (codepoint <= 0x7F) {
    utf8[0] = (uint8_t)codepoint;
    return 1;
  } else if (codepoint <= 0x7FF) {
    utf8[0] = (uint8_t)((codepoint >> 6) | 0xC0);
    utf8[1] = (uint8_t)((codepoint & 0x3F) | 0x80);
    return 2;
  } else if (codepoint <= 0xFFFF) {
    utf8[0] = (uint8_t)((codepoint >> 12) | 0xE0);
    utf8[1] = (uint8_t)(((codepoint >> 6) & 0x3F) | 0x80);
    utf8[2] = (uint8_t)((codepoint & 0x3F) | 0x80);
    return 3;
  } else if (codepoint <= 0x10FFFF) {
    utf8[0] = (uint8_t)((codepoint >> 18) | 0xF0);
    utf8[1] = (uint8_t)(((codepoint >> 12) & 0x3F) | 0x80);
    utf8[2] = (uint8_t)(((codepoint >> 6) & 0x3F) | 0x80);
    utf8[3] = (uint8_t)((codepoint & 0x3F) | 0x80);
    return 4;
  }
  return 0;
}

toke_error_z
toke_model_add_codepoint(toke_model_z* self, const uint32_t codepoint)
{
  uint8_t buf[4] = { 0, 0, 0, 0 };

  const size_t size = to_utf8(codepoint, buf);
  if (size == 0) {
    return TOKE_ERROR_INVALID_UNICODE;
  }

  return toke_model_add_token(self, buf, size);
}

toke_error_z
toke_model_add_unicode_block(toke_model_z* self, const toke_unicode_block_z block)
{
  toke_error_z err = TOKE_ERROR_NONE;

  switch (block) {
    case TOKE_UNICODE_BLOCK_BASIC_LATIN:
      for (uint32_t codepoint = 0; codepoint <= 0x007f; codepoint++) {
        err = toke_model_add_codepoint(self, codepoint);
        if (err != TOKE_ERROR_NONE) {
          break;
        }
      }
      break;
    case TOKE_UNICODE_BLOCK_GENERAL_PUNCTUATION:
      for (uint32_t codepoint = 0x2000; codepoint <= 0x206f; codepoint++) {
        err = toke_model_add_codepoint(self, codepoint);
        if (err != TOKE_ERROR_NONE) {
          break;
        }
      }
      break;
  }

  return err;
}

const uint8_t*
toke_model_get_def(const toke_model_z* self, const size_t index)
{
  return (index < self->num_defs) ? self->defs[index].data : NULL;
}

size_t
toke_model_get_size(const toke_model_z* self, const size_t index)
{
  return (index < self->num_defs) ? self->defs[index].size : 0;
}
