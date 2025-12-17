#pragma once

#include <toke/error.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct toke_filter toke_filter_z;

  toke_filter_z* toke_filter_new();

  void toke_filter_delete(toke_filter_z* self);

  toke_error_z toke_filter_parse_config(toke_filter_z* self, const char* config, size_t length);

  char* toke_filter(toke_filter_z* filter, const char* input, size_t length, size_t* out_length_ptr);

#ifdef __cplusplus
} /* extern "C" */
#endif
