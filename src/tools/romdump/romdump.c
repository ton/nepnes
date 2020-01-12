#include "ines.h"
#include "io.h"
#include "tools/romdump/options.h"
#include "util.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv)
{
  struct Options options;
  init_options(&options);
  parse_options(&options, argc, argv);

  unsigned char* rom_data = NULL;
  size_t rom_size = 0;
  if (read_all(options.rom_file_name, &rom_data, &rom_size) == -1)
  {
    quit_strerror("Could not open the given ROM file '%s' for reading", options.rom_file_name);
  }

  write_rom_information(stdout, rom_data, rom_size);

  // Note; data is not free()'d, the OS will take care of it.
  exit(0);
}
