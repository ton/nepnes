#ifndef NEPNES_IO_H
#define NEPNES_IO_H

#include <stddef.h>
#include <stdint.h>

int nn_read_all(const char *file_name, uint8_t **data, size_t *size);

#endif
