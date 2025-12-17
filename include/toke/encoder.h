#pragma once

#include <toke/error.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct toke_encoder toke_encoder_z;

  toke_encoder_z* toke_encoder_new();

  void toke_encoder_delete(toke_encoder_z* self);

  toke_error_z toke_encoder_load_vocab(toke_encoder_z* self, const char* filename);

  toke_error_z toke_encoder_parse_vocab(toke_encoder_z* self, const char* vocab, size_t length);

  uint16_t* toke_encode(toke_encoder_z* self, const void* text, size_t length, size_t* out_length);

#ifdef __cplusplus
} /* extern "C" */
#endif
