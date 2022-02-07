#include "mapper.h"

#include <string.h>

/*
 * Maps the first 16KB of ROM to 0x8000-0xbfff, and mirrors it over
 * 0xc000-0xffff in case of NROM-128. In case of NROM-256, the last 16KB of ROM
 * is mapped at 0xc000-0xffff.
 */
static int nrom_initialize_cpu(struct Cpu *cpu, uint8_t *prg_data, size_t prg_size)
{
  if (prg_size != 0x4000 && prg_size != 0x8000)  // 16Kb or 32Kb
  {
    return MAPPER_ERR_NROM_UNEXPECTED_PRG_SIZE;
  }

  const uint8_t* first_16kb = prg_data;
  const uint8_t* last_16kb = prg_size == 0x4000 ? prg_data : prg_data + 0x4000;

  memcpy(cpu->ram + 0x8000, first_16kb, 0x4000);
  memcpy(cpu->ram + 0xc000, last_16kb, 0x4000);

  cpu->PC = 0x8000;

  return 0;
}

int mapper_initialize_cpu(enum Mapper mapper, struct Cpu *cpu, uint8_t *prg_data, size_t prg_size)
{
  switch (mapper)
  {
    case MAPPER_NROM:
      return nrom_initialize_cpu(cpu, prg_data, prg_size);
      break;
    default:
      return MAPPER_ERR_UNSUPPORTED;
  }
}
