#include <toke/error.h>

const char*
toke_strerror(const toke_error_z err)
{
  switch (err) {
    case TOKE_ERROR_NONE:
      return "";
    case TOKE_ERROR_FILE_IO:
      return "file IO error";
    case TOKE_ERROR_FILE_NOT_FOUND:
      return "file not found";
    case TOKE_ERROR_MEMORY_ALLOCATION:
      return "memory allocation error";
    case TOKE_ERROR_VOCAB_SYNTAX:
      return "vocab syntax error";
    case TOKE_ERROR_FILTER_SYNTAX:
      return "filter syntax error";
    case TOKE_ERROR_INVALID_UNICODE:
      return "invalid unicode";
  }

  return "unknown error";
}
