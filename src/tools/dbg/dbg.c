#include "dbg.h"

#include "cpu.h"

/*
 * Scrolls the assembly viewport by the given number of lines. Requires the CPU
 * state to convert an assembly line number to an address in memory.
 */
void dbg_scroll_assembly(struct Debugger *debugger, struct Cpu *cpu, int lines)
{
  int current_line = cpu_instruction_count(cpu, debugger->assembly_address);
  int line = current_line + lines;
  if (line < 0) line = 0;

  debugger->assembly_address = cpu_find_instruction_address(cpu, line);
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
  debugger->assembly_address =
      cpu_find_instruction_address(cpu, cpu_instruction_count(cpu, address));
}
