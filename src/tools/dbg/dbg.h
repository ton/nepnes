#ifndef NEPNES_DBG_H
#define NEPNES_DBG_H

#include <stdint.h>

struct Cpu;

typedef uint16_t Address;

/* Stores state of the debugger. */
struct Debugger
{
  Address assembly_address; /* first address in the assembly window */
};

void dbg_scroll_assembly(struct Debugger *debugger, struct Cpu *cpu, int lines);
void dbg_scroll_assembly_to_address(struct Debugger *debugger, struct Cpu *cpu,
                                    Address address);

#endif
