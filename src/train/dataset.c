#include <toke/train/dataset.h>

#include "memmap.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#define TAR_MAGIC "ustar\0"

#define BLOCK_SIZE 512

#define MAX_UNICODE_CODEPOINTS 0x10FFFF

struct toke_dataset
{
  toke_memmap_z* file;
};

toke_dataset_z*
toke_dataset_new()
{
  toke_dataset_z* self = malloc(sizeof(toke_dataset_z));
  if (!self) {
    return NULL;
  }
  self->file = NULL;
  return self;
}

void
toke_dataset_delete(toke_dataset_z* self)
{
  if (self) {

    if (self->file) {
      toke_memmap_close(self->file);
    }
  }

  free(self);
}

toke_error_z
toke_dataset_open(toke_dataset_z* self, const char* filename)
{
  toke_memmap_z* file = toke_memmap_open(filename);
  if (!file) {
    return TOKE_ERROR_FILE_NOT_FOUND;
  }

  if (self->file) {
    toke_memmap_close(self->file);
  }

  self->file = file;

  return TOKE_ERROR_NONE;
}

static size_t
parse_size(const char* octets)
{
  size_t result = 0;

  for (size_t i = 0; i < 11; i++) {

    const char value = octets[i] - '0';

    printf("%d (%d)\n", (int)value, (int)result);

    if ((value < 0) || (value > 7)) {
      return 0;
    }

    result *= 8;
    result += (size_t)value;
  }

  return result;
}

struct job
{
  const uint8_t* text;

  size_t text_size;

  size_t file_index;
};

typedef struct job job_z;

static void
run_jobs(const job_z* jobs, const size_t num_jobs, void* walker_data, toke_dataset_walker walker)
{
#pragma omp parallel for

  for (int i = 0; i < (int)num_jobs; i++) {

    const job_z* job = jobs + i;

    walker(walker_data, job->text, job->text_size, job->file_index, /*thread_index=*/i);
  }
}

toke_error_z
toke_dataset_walk(const toke_dataset_z* self, const size_t max_threads, void* walker_data, toke_dataset_walker walker)
{
  if (!self->file) {
    return TOKE_ERROR_FILE_NOT_FOUND;
  }

  job_z* jobs = malloc(max_threads * sizeof(job_z));
  if (!jobs) {
    return TOKE_ERROR_MEMORY_ALLOCATION;
  }

  size_t num_jobs = 0;

  size_t offset = 0;

  const size_t file_size = toke_memmap_size(self->file);

  const uint8_t* ptr = toke_memmap_ptr(self->file);
  if (!ptr) {
    free(jobs);
    return TOKE_ERROR_FILE_NOT_FOUND;
  }

  size_t file_index = 0;

  while (offset < file_size) {

    const size_t remaining = file_size - offset;
    if (remaining < BLOCK_SIZE) {
      break;
    }

    const uint8_t* header = ptr + offset;

    const char* magic = (const char*)(header + 257);
    if (memcmp("ustar\0", magic, 6) != 0) {
      free(jobs);
      return TOKE_ERROR_FILE_IO;
    }

    const size_t size = parse_size((const char*)(header + 124));
    if (!size) {
      free(jobs);
      return TOKE_ERROR_FILE_IO;
    }

    const size_t num_blocks = (size + (BLOCK_SIZE - 1)) / BLOCK_SIZE;

    const uint8_t type = header[156];

    if ((type == 0) || (type == '0')) {

      // this is a regular file, add it to the queue

      job_z* next_job = &jobs[num_jobs];
      next_job->file_index = file_index;
      next_job->text = ptr + offset + BLOCK_SIZE;
      next_job->text_size = size;

      num_jobs++;

      if (num_jobs == max_threads) {
        run_jobs(jobs, num_jobs, walker_data, walker);
        num_jobs = 0;
      }
    }

    offset += /* header_block + file_data_blocks */ (1 + num_blocks) * BLOCK_SIZE;

    printf("%.*s (size_str=%.*s) (size=%d)\n", (int)6, magic, (int)12, (const char*)(header + 124), (int)size);
  }

  if (num_jobs > 0) {
    run_jobs(jobs, num_jobs, walker_data, walker);
  }

  free(jobs);

  return TOKE_ERROR_NONE;
}
