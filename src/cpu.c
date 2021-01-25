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
    case 0x10:
      // BPL - Branch if Positive
      //
      // If the negative flag is clear then add the relative displacement to the
      // program counter to cause a branch to a new location.
      if (!(cpu->P & FLAGS_NEGATIVE))
      {
        cpu->PC += instruction.bytes + cpu->ram[cpu->PC + 1];
        cpu->cycle++;
      }
      else
      {
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x18:
      // CLC - Clear Carry Flag
      //
      // Sets the carry flag to zero.
      cpu->P &= ~FLAGS_CARRY;
      cpu->PC += instruction.bytes;
      break;
    case 0x20:
      // JSR - Jump to Subroutine
      //
      // The JSR instruction pushes the address (minus one) of the return point
      // on to the stack and then sets the program counter to the target memory
      // address.

      // Push next instruction address (-1) onto the stack.
      memcpy(cpu->ram + cpu->S, &cpu->PC, 2);
      cpu->S -= 2;
      cpu->PC = *(uint16_t *)(cpu->ram + cpu->PC + 1);
      break;
    case 0x24:
      // BIT - Bit Test (zero page)
      //
      // This instructions is used to test if one or more bits are set in a
      // target memory location. The mask pattern in A is AND'ed with the value
      // in memory to set or clear the zero flag, but the result is not kept.
      // Bits 7 and 6 of the value from memory are copied into the N and V
      // flags.
      {
        uint8_t address = cpu->ram[cpu->PC + 1];
        uint8_t r = (cpu->A & cpu->ram[address]);
        if (r > 0)
        {
          cpu->P &= ~FLAGS_ZERO;
          if (r & FLAGS_OVERFLOW)
          {
            cpu->P |= FLAGS_OVERFLOW;
          }
          if (r & FLAGS_NEGATIVE)
          {
            cpu->P |= FLAGS_NEGATIVE;
          }
        }
        else
        {
          cpu->P |= FLAGS_ZERO;
          cpu->P &= ~(FLAGS_OVERFLOW | FLAGS_NEGATIVE);
        }
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x38:
      // SEC - Set Carry Flag
      //
      // Sets the carry flag to one.
      cpu->P |= FLAGS_CARRY;
      cpu->PC += instruction.bytes;
      break;
    case 0x4C:
      // JMP - Jump (absolute)
      //
      // Sets the program counter to the address specified by the operand.
      cpu->PC = *(uint16_t *)(cpu->ram + cpu->PC + 1);
      break;
    case 0x50:
      // BVC - Branch if Overflow Clear
      //
      // If the overflow flag is clear then add the relative displacement to the
      // program counter to cause a branch to a new location.
      if (!(cpu->P & FLAGS_OVERFLOW))
      {
        cpu->PC += instruction.bytes + cpu->ram[cpu->PC + 1];
        cpu->cycle++;
      }
      else
      {
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x60:
      // RTS - Return from Subroutine
      //
      // The RTS instruction is used at the end of a subroutine to return to the
      // calling routine. It pulls the program counter (minus one) from the
      // stack.
      cpu->PC = cpu->ram[cpu->S];
      cpu->PC += instruction.bytes;
      cpu->S += 2;
      break;
    case 0x70:
      // BVS - Branch if Overflow Set
      //
      // If the overflow flag is set then add the relative displacement to the
      // program counter to cause a branch to a new location.
      if (cpu->P & FLAGS_OVERFLOW)
      {
        cpu->PC += instruction.bytes + cpu->ram[cpu->PC + 1];
        cpu->cycle++;
      }
      else
      {
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x85:
      // STA - Store Accumulator (zero page)
      //
      // Stores the contents of the accumulator into memory.
      {
        uint8_t address = cpu->ram[cpu->PC + 1];
        cpu->ram[address] = cpu->A;
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x86:
      // STX - Store X Register (zero page)
      //
      // Stores the contents of the X register into memory.
      {
        uint8_t address = cpu->ram[cpu->PC + 1];
        cpu->ram[address] = cpu->X;
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x90:
      // BCC - Branch if Carry Clear
      //
      // If the carry flag is clear then add the relative displacement to the
      // program counter to cause a branch to a new location.
      if (!(cpu->P & FLAGS_CARRY))
      {
        cpu->PC += instruction.bytes + cpu->ram[cpu->PC + 1];
        cpu->cycle++;
      }
      else
      {
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xA2:
      // LDX - Load X Register (immediate)
      //
      // Loads a byte of memory into the X register setting the zero and
      // negative flags as appropriate.
      cpu->X = cpu->ram[cpu->PC + 1];

      // Set zero and negative flag if needed.
      if (cpu->X == 0)
      {
        cpu->P |= FLAGS_ZERO;
      }
      else
      {
        cpu->P &= ~FLAGS_ZERO;
        if (cpu->X & 0x80)
        {
          cpu->P |= FLAGS_NEGATIVE;
        }
        else
        {
          cpu->P &= ~FLAGS_NEGATIVE;
        }
      }

      cpu->PC += instruction.bytes;
      break;
    case 0xA9:
      // LDA - Load Accumulator (immediate)
      //
      // Loads a byte of memory into the accumulator setting the zero and
      // negative flags as appropriate.
      cpu->A = cpu->ram[cpu->PC + 1];

      // Set zero and negative flag if needed.
      if (cpu->A == 0)
      {
        cpu->P |= FLAGS_ZERO;
        cpu->P &= ~FLAGS_NEGATIVE;
      }
      else
      {
        cpu->P &= ~FLAGS_ZERO;
        if (cpu->A & 0x80)
        {
          cpu->P |= FLAGS_NEGATIVE;
        }
        else
        {
          cpu->P &= ~FLAGS_NEGATIVE;
        }
      }

      cpu->PC += instruction.bytes;
      break;
    case 0xB0:
      // BCS - Branch if Carry Set (relative)
      //
      // If the carry flag is set then add the relative displacement to the
      // program counter to cause a branch to a new location.
      if (cpu->P & FLAGS_CARRY)
      {
        cpu->PC += instruction.bytes + cpu->ram[cpu->PC + 1];
        cpu->cycle++;
      }
      else
      {
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xD0:
      // BNE - Branch if Not Equal (relative)
      //
      // If the zero flag is clear then add the relative displacement to the
      // program counter to cause a branch to a new location.
      if (!(cpu->P & FLAGS_ZERO))
      {
        cpu->PC += instruction.bytes + cpu->ram[cpu->PC + 1];
        cpu->cycle++;
      }
      else
      {
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xEA:
      // NOP - No Operation
      //
      // The NOP instruction causes no changes to the processor other than the
      // normal incrementing of the program counter to the next instruction.
      cpu->PC += instruction.bytes;
      break;
    case 0xF0:
      // BEQ - Branch if Equal
      //
      // If the zero flag is set then add the relative displacement to the
      // program counter to cause a branch to a new location.
      if (cpu->P & FLAGS_ZERO)
      {
        cpu->PC += instruction.bytes + cpu->ram[cpu->PC + 1];
        cpu->cycle++;
      }
      else
      {
        cpu->PC += instruction.bytes;
      }
      break;
    default:
      quit("Unknown opcode: %x", instruction.opcode);
      break;
  }

  cpu->cycle += instruction.cycles;
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
