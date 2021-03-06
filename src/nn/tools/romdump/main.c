#include "options.h"

#include <nn/nes/rom.h>
#include <nn/std/io.h>
#include <nn/std/util.h>

#include <stdlib.h>

int main(int argc, char **argv)
{
  struct Options options = {0};
  parse_options(&options, argc, argv);

  unsigned char *rom_data = NULL;
  size_t rom_size = 0;
  if (read_all(options.rom_file_name, &rom_data, &rom_size) == -1)
  {
    quit_strerror("Could not open the given ROM file '%s' for reading", options.rom_file_name);
  }

  if (write_rom_information(stdout, rom_data) != 0)
  {
    quit_strerror(
        "Could extract ROM information from the given ROM file '%s', "
        "unknown ROM format",
        options.rom_file_name);
  }

  /* Note; data is not free()'d, the OS will take care of it. */
  exit(0);
}
