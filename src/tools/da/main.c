#include "cpu.h"
#include "ines.h"
#include "io.h"
#include "options.h"
#include "util.h"

#include <stdlib.h>

void disassemble(uint8_t* prg_data, size_t prg_size)
{
  /* Address where the NES typically stores PRG data. */
  const uint16_t prg_rom = 0x8000;

  uint16_t rom_offset = prg_rom;

  uint8_t* pc = prg_data;
  uint8_t* end = prg_data + prg_size;
  while (pc < end)
  {
    struct Instruction ins = make_instruction(*pc);

    /* In case of an unknown instruction, the calculated opcode of the
     * instruction will be zero. */
    if (ins.opcode == 0)
    {
      printf("$%4X: .byte %02X\n", rom_offset, *pc);

      ++pc;
      ++rom_offset;
    }
    else
    {
      printf("$%4X: %s\n", rom_offset, operation_name(ins.op));

      pc += ins.bytes;
      rom_offset += ins.bytes;
    }
  }

  return;
}

int main(int argc, char** argv)
{
  struct Options options = {0};
  parse_options(&options, argc, argv);

  unsigned char* rom_data = NULL;
  size_t rom_size = 0;
  if (read_all(options.rom_file_name, &rom_data, &rom_size) == -1)
  {
    quit_strerror("Could not open the given ROM file '%s' for reading",
                  options.rom_file_name);
  }

  printf("ROM size: %lu bytes\n", rom_size);

  switch (get_rom_format(rom_data))
  {
    case RomFormat_iNes:
    {
      struct iNesHeader header = make_ines_header(rom_data);

      uint8_t* prg_data;
      ines_header_prg_data(header, rom_data, &prg_data);

      printf("PRG ROM size: %d bytes\n", header.prg_rom_size);
      printf("PRG offset in ROM data: %lu\n", (prg_data - rom_data));
      printf("\n");

      disassemble(prg_data, header.prg_rom_size);
    }
    break;
    case RomFormat_Nes20:
    case RomFormat_Unknown:
      quit(
          "Can not open the ROM file '%s', it is not an iNes ROM and in some "
          "unsupported format",
          options.rom_file_name);
      break;
  }

  exit(0);
}
