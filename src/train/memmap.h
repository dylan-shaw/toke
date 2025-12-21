#pragma once

#include <toke/error.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct toke_memmap toke_memmap_z;

  toke_memmap_z* toke_memmap_open(const char* filename);

  void toke_memmap_close(toke_memmap_z* self);

  void* toke_memmap_ptr(const toke_memmap_z* self);

  size_t toke_memmap_size(const toke_memmap_z* self);

#ifdef __cplusplus
} /* extern "C" */
#endif
