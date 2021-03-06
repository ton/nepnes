#ifndef NEPNES_UTIL_H
#define NEPNES_UTIL_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/time.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

/* String related functionality. */
char *nn_strcat(const char *x, const char *y);
bool nn_ends_with(const char *s, const char *needle);

/* File and directory functionality. */
char *nn_dirname(const char *filename);
int nn_mkdirs(const char *path, mode_t mode);

uint16_t ltob_uint16(uint16_t i);
uint32_t ltob_uint32(uint32_t i);

void nn_log(const char *fmt, ...);
void quit(const char *fmt, ...);
void quit_strerror(const char *fmt, ...);

typedef unsigned long long timestamp_t;
timestamp_t get_timestamp();

#endif
