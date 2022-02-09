#include "debugger.h"

#include <lib/6502/include/cpu.h>
#include <lib/6502/include/instruction.h>
#include <lib/std/include/util.h>

/*
 * Creates a debugger.
 */
struct Debugger make_debugger(Address prg_offset, size_t prg_size)
{
  struct Debugger debugger = {prg_offset, prg_size, {0}};
  debugger.breakpoints = make_flat_set(16);
  return debugger;
}

/*
 * Frees dynamically allocated memory for a debugger object.
 */
void destroy_debugger(struct Debugger *debugger)
{
  destroy_flat_set(&debugger->breakpoints);
}

/*
 * Converts a memory address to an instruction offset in memory. Thus, the given
 * address is converted to a number that indicates that the instruction that is
 * located at the address, or that overlaps the given address, is the n-th
 * instruction in memory.
 */
int debugger_address_to_instruction_offset(const struct Debugger *debugger, const struct Cpu *cpu,
                                           Address address)
{
  /* Each data memory location is displayed on a single line. In case we are in
   * the data segment, just return the address as the line number. */
  if (address <= debugger->prg_offset || debugger->prg_size == 0)
  {
    return address;
  }

  const uint8_t *first = cpu->ram + debugger->prg_offset;
  const uint8_t *last = cpu->ram + address;

  int offset = debugger->prg_offset;
  while (first < last)
  {
    first += instruction_size(*first);
    ++offset;
  }

  /* In case we jumped past the requested address, the address is contained
   * within an instruction, and we have to compensate. */
  return first > last ? offset - 1 : offset;
}

/*
 * Converts a memory address to a line number in the debugger. The line number
 * will display the data at the given address, or that overlaps the given
 * address.
 */
Address debugger_instruction_offset_to_address(const struct Debugger *debugger,
                                               const struct Cpu *cpu, uint16_t offset)
{
  /* Each data memory location is displayed on a single line. In case we are in
   * the data segment, just return the address as the line number. */
  if (offset <= debugger->prg_offset || debugger->prg_size == 0)
  {
    return offset;
  }

  const uint8_t *first = cpu->ram + debugger->prg_offset;
  const uint8_t *last = cpu->ram + sizeof(cpu->ram);

  int curr_offset = debugger->prg_offset;
  while (curr_offset < offset && first < last)
  {
    first += instruction_size(*first);
    ++curr_offset;
  }

  /* In case we jumped past the valid memory range, limit to the last address.
   * TODO(ton): incorrect...the last instruction might no be on $FFFF.
   */
  return (first < last ? first : last - 1) - cpu->ram;
}

/*
 * Returns whether a breakpoint is current set for the given address.
 */
bool debugger_has_breakpoint_at(struct Debugger *debugger, Address address)
{
  return flat_set_contains(&debugger->breakpoints, address);
}

/*
 * Toggles a breakpoint for the given memory address. In case a breakpoint is
 * already set for the given address, removes it, otherwise, sets it. Returns
 * the index of the breakpoint that was added or removed.
 */
size_t debugger_toggle_breakpoint_at(struct Debugger *debugger, Address address)
{
  return flat_set_contains(&debugger->breakpoints, address)
             ? flat_set_remove(&debugger->breakpoints, address)
             : flat_set_insert(&debugger->breakpoints, address);
}
