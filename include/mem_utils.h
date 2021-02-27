#ifndef MEM_UTILS__
#define MEM_UTILS__

#include <inttypes.h>

int read_word(uint16_t *out, uint16_t address);
int write_word(uint16_t value, uint16_t address);

#endif