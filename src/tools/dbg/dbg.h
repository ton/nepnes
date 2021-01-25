#ifndef NEPNES_DBG_H
#define NEPNES_DBG_H

#include "cpu.h"

#include <stdint.h>

/* Stores state of the debugger. */
struct Debugger
{
  int line;              /* current focused line of assembly to display */
  int last_line;         /* line number of the last line of assembly */
  uint8_t scroll_offset; /* minimal number of lines to keep above and below the
                            PC line */

  /* The following is used by the debugger which parts of the memory should be
   * interpreted as program data. This may not be sufficient! */

  uint16_t prg_offset; /* start of the program data */
  size_t prg_size;     /* size of the program data */
};

void dbg_init(struct Debugger *debugger, struct Cpu *cpu, uint16_t prg_address,
              size_t prg_data_size);

int dbg_address_to_line(struct Debugger *debugger, struct Cpu *cpu,
                        uint16_t address);
uint16_t dbg_line_to_address(struct Debugger *debugger, struct Cpu *cpu,
                             int line);

void dbg_scroll_assembly(struct Debugger *debugger, int lines);
void dbg_scroll_assembly_to_address(struct Debugger *debugger, struct Cpu *cpu,
                                    Address address);
void dbg_scroll_assembly_to_pc(struct Debugger *debugger, struct Cpu *cpu);

#endif
