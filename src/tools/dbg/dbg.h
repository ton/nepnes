#ifndef NEPNES_DBG_H
#define NEPNES_DBG_H

#include "cpu.h"
#include "flat_set.h"

#include <stdint.h>

/* Stores state of the debugger. */
struct Debugger
{
  /*
   * The following is used by the debugger to determine which parts of the
   * memory should be interpreted as program data. This may not be sufficient!
   */

  uint16_t prg_offset; /* start of the program data */
  size_t prg_size;     /* size of the program data */

  struct flat_set breakpoints;
};

struct Debugger make_debugger(Address prg_offset, size_t prg_size);
void destroy_debugger(struct Debugger *debugger);

Address dbg_instruction_offset_to_address(const struct Debugger *debugger, const struct Cpu *cpu,
                                          uint16_t offset);
int dbg_address_to_instruction_offset(const struct Debugger *debugger, const struct Cpu *cpu,
                                      Address address);

bool debugger_has_breakpoint_at(struct Debugger *debugger, Address address);
size_t debugger_toggle_breakpoint_at(struct Debugger *debugger, Address address);

#endif
