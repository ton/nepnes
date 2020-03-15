#ifndef NEPNES_UTIL_H
#define NEPNES_UTIL_H

#include <stdint.h>

char* nn_strcat(const char* x, const char* y);

uint16_t uint16_ltob(uint16_t i);
uint32_t uint32_ltob(uint32_t i);

void quit(const char* fmt, ...);
void quit_strerror(const char* fmt, ...);

#endif
