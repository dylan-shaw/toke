#pragma once

#include <toke/error.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct toke_model toke_model_z;

  toke_model_z* toke_model_new();

  void toke_model_delete(toke_model_z* self);

  size_t toke_model_size(const toke_model_z* self);

  toke_error_z toke_model_add_token(toke_model_z* self, const uint8_t* data, size_t size);

  const uint8_t* toke_model_get_def(const toke_model_z* self, const size_t index);

  size_t toke_model_get_size(const toke_model_z* self, const size_t index);

#ifdef __cplusplus
} /* extern "C" */
#endif
