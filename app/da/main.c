#include "options.h"

#include <lib/6502/include/cpu.h>
#include <lib/6502/include/da.h>
#include <lib/nes/include/rom.h>
#include <lib/std/include/io.h>
#include <lib/std/include/util.h>

#include <stdlib.h>

int main(int argc, char **argv)
{
  struct Options options = {0};
  parse_options(&options, argc, argv);

  unsigned char *rom_data = NULL;
  size_t rom_size = 0;
  if (nn_read_all(options.rom_file_name, &rom_data, &rom_size) == -1)
  {
    nn_quit_strerror("Could not open the given ROM file '%s' for reading", options.rom_file_name);
  }

  printf("ROM size: %lu bytes\n", rom_size);

  struct RomHeader header = rom_make_header(rom_data);
  if (header.rom_format == RF_UNKNOWN)
  {
    nn_quit("Can not open the ROM file '%s', unknown ROM format", options.rom_file_name);
  }

  uint8_t *prg_data;
  size_t prg_data_size;
  rom_prg_data(&header, rom_data, &prg_data, &prg_data_size);

  printf("PRG ROM size: %lu bytes\n", prg_data_size);
  printf("PRG offset in ROM data: %lu\n", (prg_data - rom_data));
  printf("\n");

  return nn_disassemble(stdout, prg_data, prg_data_size);
}
