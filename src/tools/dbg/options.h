#ifndef NEPNES_TOOLS_DBG_OPTIONS_H
#define NEPNES_TOOLS_DBG_OPTIONS_H

struct Options
{
  char *binary_file_name;
  int print_help;
};

void init_options(struct Options *options);
void parse_options(struct Options *options, int argc, char **argv);

#endif
