#include "dbg.h"

#include "cpu.h"

/*
 * Scrolls the debugger by the given number of lines.
 */
void dbg_scroll_assembly(struct Debugger *debugger, int lines)
{
  debugger->line += lines;
  if (debugger->line < 0) debugger->line = 0;
}

/*
 * Scrolls the assembly viewport to display the instruction that starts at or
 * overlaps the given address.
 */
void dbg_scroll_assembly_to_address(struct Debugger *debugger, struct Cpu *cpu,
                                    Address address)
{
  /* We can not simply assign the requested address to the debugger state; but
   * need to normalize it such that the debugger address is set to the
   * instruction that starts or overlaps the requested address. */
  Address assembly_address =
      cpu_find_instruction_address(cpu, cpu_instruction_count(cpu, address));

  debugger->line = cpu_instruction_count(cpu, address);
}
