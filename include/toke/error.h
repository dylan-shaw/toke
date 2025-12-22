#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

  enum toke_error
  {
    TOKE_ERROR_NONE,
    TOKE_ERROR_MEMORY_ALLOCATION,
    TOKE_ERROR_FILE_NOT_FOUND,
    TOKE_ERROR_FILE_IO,
    TOKE_ERROR_VOCAB_SYNTAX,
    TOKE_ERROR_FILTER_SYNTAX,
    TOKE_ERROR_INVALID_UNICODE
  };

  typedef enum toke_error toke_error_z;

  const char* toke_strerror(toke_error_z error);

#ifdef __cplusplus
} /* extern "C" */
#endif
