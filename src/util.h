#ifndef NEPNES_UTIL_H
#define NEPNES_UTIL_H

#include <stdint.h>

char *nn_strcat(const char *x, const char *y);

uint16_t ltob_uint16(uint16_t i);
uint32_t ltob_uint32(uint32_t i);

void quit(const char *fmt, ...);
void quit_strerror(const char *fmt, ...);

#endif
