#pragma once

#include <toke/error.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct toke_decoder toke_decoder_z;

  toke_decoder_z* toke_decoder_new();

  void toke_decoder_delete(toke_decoder_z* self);

  toke_error_z toke_decoder_load_vocab(toke_decoder_z* self, const char* filename);

  toke_error_z toke_decoder_parse_vocab(toke_decoder_z* self, const char* vocab, size_t length);

  char* toke_decode(toke_decoder_z* self, const uint16_t* tokens, const size_t length, size_t* out_length_ptr);

#ifdef __cplusplus
} /* extern "C" */
#endif
