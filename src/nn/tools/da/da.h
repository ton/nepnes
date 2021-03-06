#ifndef NEPNES_TOOLS_DA_DA_H
#define NEPNES_TOOLS_DA_DA_H

#include <stdint.h>
#include <stdio.h>

int da_disassemble(FILE *fp, uint8_t *prg_data, size_t prg_size);

#endif  // NEPNES_TOOLS_DA_DA_H
