#include "dbg.h"

#include "cpu.h"
#include "instruction.h"

/*
 * Initializes a debugger, given the CPU state, focuses the debugger on the
 * instruction the program counter points to.
 */
void dbg_init(struct Debugger *debugger, struct Cpu *cpu, uint16_t prg_offset,
              size_t prg_size)
{
  debugger->scroll_offset = 3;
  debugger->prg_offset = prg_offset;
  debugger->prg_size = prg_size;
  debugger->last_line = dbg_address_to_line(debugger, cpu, CPU_MAX_ADDRESS);

  dbg_scroll_assembly_to_pc(debugger, cpu);
}

/*
 * Converts a memory address to a line number in the debugger. The line number
 * will display the data at the given address, or that overlaps the given
 * address.
 */
int dbg_address_to_line(struct Debugger *debugger, struct Cpu *cpu,
                        uint16_t address)
{
  /* Each data memory location is displayed on a single line. In case we are in
   * the data segment, just return the address as the line number. */
  if (address <= debugger->prg_offset || debugger->prg_size == 0)
  {
    return address;
  }

  uint8_t *first = cpu->ram + debugger->prg_offset;
  uint8_t *last = cpu->ram + address;

  int line = debugger->prg_offset;
  while (first < last)
  {
    first += instruction_size(*first);
    ++line;
  }

  /* In case we jumped past the requested address, the address is contained
   * within an instruction, and we have to compensate. */
  return first > last ? line - 1 : line;
}

/*
 * Converts a memory address to a line number in the debugger. The line number
 * will display the data at the given address, or that overlaps the given
 * address.
 */
uint16_t dbg_line_to_address(struct Debugger *debugger, struct Cpu *cpu,
                             int line)
{
  /* Each data memory location is displayed on a single line. In case we are in
   * the data segment, just return the address as the line number. */
  if (line <= debugger->prg_offset || debugger->prg_size == 0)
  {
    return line;
  }

  uint8_t *first = cpu->ram + debugger->prg_offset;
  uint8_t *last = cpu->ram + sizeof(cpu->ram) + 1;

  int curr_line = debugger->prg_offset;
  while (curr_line < line && first < last)
  {
    first += instruction_size(*first);
    ++curr_line;
  }

  /* In case we jumped past the requested address, the address is contained
   * within an instruction, and we have to compensate. */
  return (first > last ? last : first) - cpu->ram;
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
  debugger->line =
      dbg_address_to_line(debugger, cpu, address) - debugger->scroll_offset;
  if (debugger->line < 0) debugger->line = 0;
  if (debugger->line > debugger->last_line)
    debugger->line = debugger->last_line;
}

/*
 * Scrolls the assembly viewport to display the instruction that starts at or
 * overlaps the program counter.
 */
void dbg_scroll_assembly_to_pc(struct Debugger *debugger, struct Cpu *cpu)
{
  dbg_scroll_assembly_to_address(debugger, cpu, cpu->PC);
}
