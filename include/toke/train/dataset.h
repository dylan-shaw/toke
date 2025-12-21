#pragma once

#include <toke/error.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef void (*toke_dataset_walker)(void* user_data, const uint8_t* text, size_t text_size, size_t file_index);

  typedef struct toke_dataset toke_dataset_z;

  toke_dataset_z* toke_dataset_new();

  void toke_dataset_delete(toke_dataset_z* self);

  toke_error_z toke_dataset_open(toke_dataset_z* self, const char* filename);

  toke_error_z toke_dataset_walk(const toke_dataset_z* self,
                                 void* walker_data,
                                 toke_dataset_walker walker,
                                 size_t max_threads);

#ifdef __cplusplus
} /* extern "C" */
#endif
