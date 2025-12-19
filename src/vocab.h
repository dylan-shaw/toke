#pragma once

#include <stddef.h>
#include <stdint.h>

uint8_t*
toke_process_token_def(const char* word, const size_t word_len, size_t* out_length);
