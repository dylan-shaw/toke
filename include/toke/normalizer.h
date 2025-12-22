#pragma once

#include <toke/error.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct toke_normalizer toke_normalizer_z;

  toke_normalizer_z* toke_normalizer_new();

  void toke_normalizer_delete(toke_normalizer_z* self);

  toke_error_z toke_normalizer_parse_config(toke_normalizer_z* self, const char* config, size_t length);

  char* toke_normalize(toke_normalizer_z* self, const char* input, size_t length, size_t* out_length_ptr);

#ifdef __cplusplus
} /* extern "C" */
#endif
