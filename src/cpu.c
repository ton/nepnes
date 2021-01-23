#include "cpu.h"

#include "instruction.h"

#include <string.h>

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

/*
 * Initializes the CPU to its initial state after power on (for a NES).
 */
void cpu_power_on(struct Cpu *cpu)
{
  cpu->A = 0;
  cpu->X = 0;
  cpu->Y = 0;
  cpu->S = 0xfd;
  cpu->P = 0x34;

  cpu->ram[0x4015] = 0x00;             /* all channels disabled */
  cpu->ram[0x4017] = 0x00;             /* frame IRQ disabled */
  memset(cpu->ram + 0x4000, 0x00, 16); /* 0x4000-0x400f: 0x00 */
  memset(cpu->ram + 0x4010, 0x00, 4);  /* 0x4000-0x400f: 0x00 */
}

/*
 * Initializes the CPU to its documented state after a reset (for a NES).
 */
void cpu_power_reset(struct Cpu *cpu)
{
  cpu->S -= 3;
  cpu->P |= FLAGS_INTERRUPT_DISABLE;
}
