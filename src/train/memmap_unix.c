#include "memmap.h"

#include <stdlib.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

struct toke_memmap
{
  int fd;

  struct stat stbuf;

  void* ptr;
};

toke_memmap_z*
toke_memmap_open(const char* filename)
{
  toke_memmap_z* self = malloc(sizeof(toke_memmap_z));
  if (!self) {
    return NULL;
  }

  self->fd = open(filename, O_RDONLY);
  if (self->fd == -1) {
    free(self);
    return NULL;
  }

  if (fstat(self->fd, &self->stbuf) == -1) {
    close(self->fd);
    free(self);
    return NULL;
  }

  self->ptr = mmap(NULL, self->stbuf.st_size, PROT_READ, MAP_PRIVATE, self->fd, 0);
  if (self->ptr == MAP_FAILED) {
    close(self->fd);
    free(self);
    return NULL;
  }

  return self;
}

void
toke_memmap_close(toke_memmap_z* self)
{
  if (self) {
    close(self->fd);
  }
  free(self);
}

void*
toke_memmap_ptr(const toke_memmap_z* self)
{
  return self->ptr;
}

size_t
toke_memmap_size(const toke_memmap_z* self)
{
  return self->stbuf.st_size;
}
