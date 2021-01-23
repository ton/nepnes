#ifndef NEPNES_DBG_H
#define NEPNES_DBG_H

#include "cpu.h"

#include <stdint.h>

/* Stores state of the debugger. */
struct Debugger
{
  int line;          /* current focused line of assembly to display */
  int pc_line;       /* line of assembly the PC is pointing to */
  int last_line;     /* line number of the last line of assembly */
  int scroll_offset; /* minimal number of lines to keep above and below the PC
                        line */
};

void dbg_init(struct Debugger *debugger, struct Cpu *cpu);
void dbg_scroll_assembly(struct Debugger *debugger, int lines);
void dbg_scroll_assembly_to_address(struct Debugger *debugger, struct Cpu *cpu,
                                    Address address);
void dbg_scroll_assembly_to_pc(struct Debugger *debugger, struct Cpu *cpu);

#endif
