#include "options.h"

#include <nn/6502/cpu.h>
#include <nn/std/util.h>

#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage()
{
  printf("Usage: dbg -i|--input BINARY [-a|--address ADDRESS] [-h|--help]\n");
}

static void print_help()
{
  printf("dbg - command-line debugger for the NMOS 6502 binaries\n\n");
  print_usage();
  printf("\n");
  printf("\t-i BINARY     : BINARY file to debug\n");
  printf(
      "\t-a ADDRESS    : hexadecimal ADDRESS in memory where to load the "
      "contents of a BINARY file\n");
  printf(
      "\t-l LOGFILE    : Outputs CPU state to the given log file for every "
      "instruction\n");
  printf("\t-h | --help   : shows this help message\n");
}

/*
 * Initializes the options struct, by default the starting address is set to an
 * invalid address to distinguish it from user input.
 */
void options_init(struct Options* options)
{
  options->binary_file_name = NULL;
  options->log_file_name = NULL;
  options->print_help = false;
  options->address = CPU_MAX_ADDRESS;
}

void options_parse(struct Options *options, int argc, char **argv)
{
  struct option opts[] = {
      {"help", no_argument, NULL, 'h'},
      {"input", required_argument, NULL, 'i'},
      {"address", required_argument, NULL, 'a'},
      {"log", optional_argument, NULL, 'l'},
  };

  if (argc == 1)
  {
    print_usage();
    exit(1);
  }

  int option_index = 0;
  char ch;
  while ((ch = getopt_long(argc, argv, "hi:a:l:", opts, &option_index)) != -1)
  {
    switch (ch)
    {
      case 'h':
        print_help();
        exit(1);
        break;
      case 'a':
        sscanf(optarg, "%" SCNx16, &options->address);
        break;
      case 'i':
        options->binary_file_name = strdup(optarg);
        break;
      case 'l':
        options->log_file_name = strdup(optarg);
        break;
    }
  }

  if (options->binary_file_name == NULL)
  {
    nn_quit("Missing required argument: -i BINARY");
  }
}
