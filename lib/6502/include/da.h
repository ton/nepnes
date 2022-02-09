#ifndef NEPNES_6502_DA_H
#define NEPNES_6502_DA_H

#include <stdint.h>
#include <stdio.h>

int nn_disassemble(FILE *fp, uint8_t *prg_data, size_t prg_size);

#endif
