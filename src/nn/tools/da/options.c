#include "options.h"

#include <nn/std/util.h>

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage()
{
  printf("Usage: da -i|--input ROMFILE [-h|--help]\n");
}

static void print_help()
{
  printf("da - disassembler for NES roms\n\n");
  print_usage();
  printf("\n");
  printf("\t-i ROMFILE     : ROM file to disassemble\n");
  printf("\t-h | --help    : shows this help message\n");
}

void parse_options(struct Options *options, int argc, char **argv)
{
  struct option opts[] = {
      {"help", no_argument, NULL, 'h'},
      {"input", required_argument, NULL, 'i'},
  };

  if (argc == 1)
  {
    print_usage();
    exit(1);
  }

  int option_index = 0;
  char ch;
  while ((ch = getopt_long(argc, argv, "hi:", opts, &option_index)) != -1)
  {
    switch (ch)
    {
      case 'h':
        print_help();
        exit(1);
        break;
      case 'i':
        options->rom_file_name = strdup(optarg);
        break;
    }
  }

  if (options->rom_file_name == NULL)
  {
    quit("Missing required argument: -i ROMFILE");
  }
}
