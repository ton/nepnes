#include "dbg.h"

#include "cpu.h"

/*
 * Initializes a debugger, given the CPU state, focuses the debugger on the
 * instruction the program counter points to.
 */
void dbg_init(struct Debugger *debugger, struct Cpu *cpu)
{
  debugger->pc_line = cpu_instruction_count(cpu, cpu->PC);
  debugger->last_line = cpu_instruction_count(cpu, CPU_MAX_ADDRESS);
  dbg_scroll_assembly_to_address(debugger, cpu, cpu->PC);
}

/*
 * Scrolls the debugger by the given number of lines.
 */
void dbg_scroll_assembly(struct Debugger *debugger, int lines)
{
  debugger->line += lines;
  if (debugger->line < 0) debugger->line = 0;
  if (debugger->line > debugger->last_line)
    debugger->line = debugger->last_line;
}

/*
 * Scrolls the assembly viewport to display the instruction that starts at or
 * overlaps the given address.
 */
void dbg_scroll_assembly_to_address(struct Debugger *debugger, struct Cpu *cpu,
                                    Address address)
{
  debugger->line = cpu_instruction_count(cpu, address);
}
