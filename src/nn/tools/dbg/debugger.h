#ifndef NEPNES_DEBUGGER_H
#define NEPNES_DEBUGGER_H

#include <nn/6502/cpu.h>
#include <nn/std/flat_set.h>

#include <stdint.h>

/* Stores state of the debugger. */
struct debugger
{
  /*
   * The following is used by the debugger to determine which parts of the
   * memory should be interpreted as program data. This may not be sufficient!
   */

  uint16_t prg_offset; /* start of the program data */
  size_t prg_size;     /* size of the program data */

  struct flat_set breakpoints;
};

struct debugger make_debugger(Address prg_offset, size_t prg_size);
void destroy_debugger(struct debugger *debugger);

Address debugger_instruction_offset_to_address(const struct debugger *debugger,
                                               const struct cpu *cpu, uint16_t offset);
int debugger_address_to_instruction_offset(const struct debugger *debugger, const struct cpu *cpu,
                                           Address address);

bool debugger_has_breakpoint_at(struct debugger *debugger, Address address);
size_t debugger_toggle_breakpoint_at(struct debugger *debugger, Address address);

#endif
