#ifndef NEPNES_APP_DBG_OPTIONS_H
#define NEPNES_APP_DBG_OPTIONS_H

#include <stdbool.h>

#include "debugger.h"

struct Options
{
  Address address;
  char *binary_file_name;
  char *log_file_name;
  bool print_help;
};

void options_init(struct Options *options);
void options_parse(struct Options *options, int argc, char **argv);

#endif
