#include "cpu.h"
#include "ines.h"
#include "io.h"
#include "options.h"
#include "util.h"

#include <stdlib.h>

void disassemble(uint8_t* prg_data, size_t prg_size)
{
  short int rom = 0x8000;
  uint8_t* pc = prg_data;
  uint8_t* end = prg_data + 1;
  while (pc != end)
  {
    // struct Opcode opcode = make_opcode(*pc);
    printf("%hx: %hhx %s\n", rom, *pc, "unknown");
    ++pc;
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

  switch (get_rom_format(rom_data))
  {
    case RomFormat_iNes:
    {
      struct iNesHeader header = make_ines_header(rom_data);

      uint8_t* prg_data;
      ines_header_prg_data(header, rom_data, &prg_data);
      disassemble(prg_data, header.prg_rom_size);

      printf("PRG data at: $%04lu\n", (prg_data - rom_data));
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
