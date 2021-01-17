#include "cpu.h"

#include "instruction.h"

/*
 * Returns the number of instructions starting from the first address in RAM,
 * up to the given address.
 */
int cpu_instruction_count(struct Cpu *cpu, Address address)
{
  uint8_t *first = cpu->ram;
  uint8_t *last = cpu->ram + address;

  int count = 0;
  while (first < last)
  {
    first += instruction_size(*first);
    ++count;
  }

  /* In case we jumped past the requested address, the address is contained
   * within an instruction, and we have to compensate. */
  return first > last ? count - 1 : count;
}

/*
 * Returns the address of the n-th instruction in memory.
 */
uint16_t cpu_find_instruction_address(struct Cpu *cpu, int n)
{
  uint8_t *pc = cpu->ram;

  int i = 0;
  while (i++ < n)
  {
    pc += instruction_size(*pc);
  }

  return (pc - cpu->ram);
}
