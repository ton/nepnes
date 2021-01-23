#include "cpu.h"

#include "instruction.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

/*
 * Executes the instruction currently pointed to by the program counter register
 * (PC). Updates register state, updates cycle count.
 */
void cpu_execute_next_instruction(struct Cpu *cpu)
{
  const struct Instruction instruction = make_instruction(cpu->ram[cpu->PC]);

  switch (instruction.opcode)
  {
    case 0:
      // TODO(ton): invalid opcode; what to do?
      return;
    case 0x20:
      // JSR - absolute
      cpu->PC = *(uint16_t *)(cpu->ram + cpu->PC + 1);

      // Push next instruction address (-1) onto the stack.
      cpu->S -= 2;
      break;
    case 0x38:
      // SEC
      cpu->P |= FLAGS_CARRY;
      cpu->PC += instruction.bytes;
      break;
    case 0x4C:
      // JMP - absolute
      cpu->PC = *(uint16_t *)(cpu->ram + cpu->PC + 1);
      break;
    case 0x86:
      // STX - zero page
      {
        uint8_t address = cpu->ram[cpu->PC + 1];
        cpu->ram[address] = cpu->X;
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xA2:
      // LDX - immediate
      cpu->X = cpu->ram[cpu->PC + 1];

      // Set zero and negative flag if needed.
      cpu->P |= (cpu->X == 0 ? FLAGS_ZERO : FLAGS_NONE);
      cpu->P |= (cpu->X & 0x80 ? FLAGS_NEGATIVE : FLAGS_NONE);

      cpu->PC += instruction.bytes;
      break;
    case 0xB0:
      // BCS - relative
      if (cpu->P & FLAGS_CARRY)
      {
        cpu->PC += instruction.bytes + cpu->ram[cpu->PC + 1];
        cpu->cycle++;
      }
      break;
    case 0xEA:
      cpu->PC += instruction.bytes;
      break;
    default:
      quit("Unknown opcode: %x", instruction.opcode);
      break;
  }

  cpu->cycle += instruction.cycles;
}

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
  cpu->P = 0x24; /* nesdev wiki says $34, set to $24 for now to equal
                    Nintendulator */

  cpu->ram[0x4015] = 0x00;             /* all channels disabled */
  cpu->ram[0x4017] = 0x00;             /* frame IRQ disabled */
  memset(cpu->ram + 0x4000, 0x00, 16); /* 0x4000-0x400f: 0x00 */
  memset(cpu->ram + 0x4010, 0x00, 4);  /* 0x4000-0x400f: 0x00 */

  cpu->cycle = 7;
}

/*
 * Initializes the CPU to its documented state after a reset (for a NES).
 */
void cpu_power_reset(struct Cpu *cpu)
{
  cpu->S -= 3;
  cpu->P |= FLAGS_INTERRUPT_DISABLE;
  cpu->cycle += 7;
}
