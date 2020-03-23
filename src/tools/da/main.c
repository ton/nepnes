#include "cpu.h"
#include "da.h"
#include "io.h"
#include "options.h"
#include "rom.h"
#include "util.h"

#include <stdlib.h>

int main(int argc, char **argv)
{
  struct Options options = {0};
  parse_options(&options, argc, argv);

  unsigned char *rom_data = NULL;
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
      uint8_t *prg_data;
      size_t prg_data_size;
      rom_prg_data(&header, rom_data, &prg_data, &prg_data_size);

      printf("PRG ROM size: %lu bytes\n", prg_data_size);
      printf("PRG offset in ROM data: %lu\n", (prg_data - rom_data));
      printf("\n");

      da_disassemble(stdout, prg_data, prg_data_size);
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
