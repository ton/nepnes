#include "cpu.h"
#include "io.h"
#include "options.h"
#include "rom.h"
#include "util.h"

#include <stdlib.h>

void disassemble(uint8_t* prg_data, size_t prg_size)
{
  /* Address where the NES typically stores PRG data. */
  const uint32_t prg_rom = 0x08000;

  uint32_t rom_offset = prg_rom;

  uint8_t* pc = prg_data;
  uint8_t* end = prg_data + prg_size;
  while (pc < end)
  {
    struct Instruction ins = make_instruction(*pc);

    const int assembly_size = 14;

    /* In case of an unknown instruction, the calculated opcode of the
     * instruction will be zero. */
    if (ins.bytes == 0)
    {
      printf("$%X: %*s (%02X)\n", rom_offset, assembly_size, "", *pc);

      ++pc;
      ++rom_offset;
    }
    else
    {
      uint32_t encoding = *pc;
      for (int i = 1; i < ins.bytes; ++i)
      {
        encoding = (encoding << 8) + (pc + i < end ? *(pc + i) : 0);
      }

      char assembly[assembly_size + 1];
      Instruction_print(assembly, sizeof assembly, &ins, encoding);

      printf("$%X: %-*s (%0*X)\n", rom_offset, assembly_size, assembly,
             ins.bytes * 2, encoding);

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

  struct RomHeader header = rom_make_header(rom_data);
  switch (header.rom_format)
  {
    case RomFormat_iNes:
    case RomFormat_Nes20:
    {
      uint8_t* prg_data;
      rom_prg_data(header, rom_data, &prg_data);

      printf("PRG ROM size: %d bytes\n", rom_size_in_bytes(&header));
      printf("PRG offset in ROM data: %lu\n", (prg_data - rom_data));
      printf("\n");

      disassemble(prg_data, rom_size_in_bytes(&header));
    }
    break;
    case RomFormat_Unknown:
      quit(
          "Can not open the ROM file '%s', it is not an iNes ROM and in some "
          "unsupported format",
          options.rom_file_name);
      break;
  }

  exit(0);
}
