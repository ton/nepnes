#ifndef NN_TOOLS_DBG_OPTIONS_H
#define NN_TOOLS_DBG_OPTIONS_H

#include "debugger.h"

struct Options
{
  Address address;
  char *binary_file_name;
  char *log_file_name;
  int print_help;
};

void init_options(struct Options *options);
void parse_options(struct Options *options, int argc, char **argv);

#endif
