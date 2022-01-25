#include "options.h"

#include <nn/std/util.h>

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage()
{
  printf("Usage: romdump -i|--input ROMFILE [-h|--help]\n");
}

static void print_help()
{
  printf("romdump - outputs iNES ROM information to standard output\n\n");
  print_usage();
  printf("\n");
  printf("\t-i ROMFILE     : ROM file to disassemble\n");
  printf("\t-h | --help    : shows this help message\n");
}

void init_options(struct Options *options)
{
  memset(options, 0, sizeof(struct Options));
  options->print_help = 0;
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
  while ((ch = getopt_long(argc, argv, "hvi:", opts, &option_index)) != -1)
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
    nn_quit("Missing required argument: -i ROMFILE");
  }
}
