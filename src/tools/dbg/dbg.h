#ifndef NEPNES_DBG_H
#define NEPNES_DBG_H

#include <stdint.h>

struct Cpu;

typedef uint16_t Address;

/* Stores state of the debugger. */
struct Debugger
{
  int line;    /* current focused line of assembly to display */
  int pc_line; /* line of assembly the PC is pointing to */
};

int dbg_address_to_line(struct Cpu *cpu, Address address);

void dbg_scroll_assembly(struct Debugger *debugger, int lines);
void dbg_scroll_assembly_to_address(struct Debugger *debugger, struct Cpu *cpu,
                                    Address address);

#endif
