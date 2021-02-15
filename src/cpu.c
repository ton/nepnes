#include "cpu.h"

#include "instruction.h"
#include "util.h"

#include <stdio.h>
#include <string.h>

#define STACK_OFFSET 0x0100

/*
 * Clears bit `n` in `x`.
 */
#define BIT_CLEAR(x, n) (x &= ~(1 << n))
/*
 * Sets bit `n` in `x`.
 */
#define BIT_SET(x, n) (x |= (1 << n))
/*
 * In case `p` is 1, will set bit `n` in `x` to 1, otherwise sets bit `n` in `x`
 * to 0.
 */
#define BIT_SET_IF(p, x, n) (x = (x & ~(1 << n)) | (((p) > 0) << n))

/*
 * Pops an 8-bit value from the stack.
 */
static uint8_t cpu_pop_8b(struct Cpu *cpu)
{
  cpu->S += 1;
  return cpu->ram[STACK_OFFSET + cpu->S];
}

/*
 * Pushes an 8-bit value to the stack.
 */
static void cpu_push_8b(struct Cpu *cpu, uint8_t i)
{
  cpu->ram[STACK_OFFSET + cpu->S] = i;
  --cpu->S;
}

/*
 * Pops a 16-bit value from the stack.
 */
static uint16_t cpu_pop_16b(struct Cpu *cpu)
{
  uint16_t i;
  cpu->S += 2;
  memcpy(&i, cpu->ram + STACK_OFFSET + cpu->S, 2);
  return i;
}

/*
 * Pushes a 16-bit value on to the stack.
 */
static void cpu_push_16b(struct Cpu *cpu, uint16_t i)
{
  memcpy(cpu->ram + STACK_OFFSET + cpu->S, &i, 2);
  cpu->S -= 2;
}

/*
 * Depending on the given value `x`, sets the zero and negative CPU flags
 * accordingly. The zero flag is set in case the value in the accumulator is
 * zero. The negative flag is set in case bit 7 of the accumulator is set.
 */
static void cpu_set_zero_negative_flags(struct Cpu *cpu, uint8_t x)
{
  BIT_SET_IF(x == 0, cpu->P, FLAGS_BIT_ZERO);
  BIT_SET_IF(x & 0x80, cpu->P, FLAGS_BIT_NEGATIVE);
}

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
      /* TODO(ton): invalid opcode; what to do? */
      return;
    case 0x08:
      /*
       * PHP - Push Processor State
       *
       * Pushes a copy of the status flags on to the stack. Note; in this case
       * the B-flag (bit 4 and 5 in the status flag) is also set. Bit 5 is
       * always set when pushed to the stack, bit 4 is set because it is pushed
       * as a result of being pushed using PHP.
       */
      cpu_push_8b(cpu, cpu->P | FLAGS_BRK_PHP_PUSH);
      cpu->PC += instruction.bytes;
      break;
    case 0x09:
      /*
       * ORA - Logical Inclusive OR (immediate)
       *
       * An exclusive OR is performed, bit by bit, on the contents of the
       * accumulator and some operand value, and stores the result in the
       * accumulator. Updates the zero and negative flags accordingly.
       */
      cpu->A |= cpu->ram[cpu->PC + 1];
      cpu_set_zero_negative_flags(cpu, cpu->A);
      cpu->PC += instruction.bytes;
      break;
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
      cpu_push_16b(cpu, cpu->PC + instruction.bytes - 1);
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
        const uint8_t address = cpu->ram[cpu->PC + 1];
        const uint8_t value = cpu->ram[address];
        BIT_SET_IF(!(cpu->A & value), cpu->P, FLAGS_BIT_ZERO);
        cpu->P = (cpu->P & 0x3f) | (value & 0xc0);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x28:
      /*
       * PLP - Pull Processor Status
       *
       * Pulls an 8bit value from the stack and into the processor flags.
       * Ignores the 'B-flag', bits 4 and 5.
       */
      cpu->P = (cpu_pop_8b(cpu) & 0xcf) | (cpu->P & 0x30);
      cpu->PC += instruction.bytes;
      break;
    case 0x29:
      /*
       * AND - Logical And (immediate)
       *
       * A logical AND is performed, bit by bit, on the accumulator
       * contents using the contents of a byte of memory.
       */
      cpu->A &= cpu->ram[cpu->PC + 1];
      cpu_set_zero_negative_flags(cpu, cpu->A);
      cpu->PC += instruction.bytes;
      break;
    case 0x30:
      /*
       * BMI - Branch If Minus (relative)
       *
       * If the negative flag is set, then add the relative displacement to the
       * program counter to cause a branch to a new location.
       */
      /* TODO(ton): implement. */
      cpu->PC += instruction.bytes;
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
    case 0x48:
      /*
       * PHA - Push Accumulator
       *
       * Pushes a copy of the accumulator on to the stack.
       */
      cpu_push_8b(cpu, cpu->A);
      cpu->PC += instruction.bytes;
      break;
    case 0x49:
      /*
       * EOR - Exclusive OR (immediate)
       *
       * Performs an exclusive OR, bit by bit, on the value in the accumulator
       * and the given operand, and stores the result in the accumulator. The
       * zero and negative flags are set accordingly.
       */
      cpu->A ^= cpu->ram[cpu->PC + 1];
      cpu_set_zero_negative_flags(cpu, cpu->A);
      cpu->PC += instruction.bytes;
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
      cpu->PC = cpu_pop_16b(cpu);
      cpu->PC += instruction.bytes;
      break;
    case 0x68:
      // PLA - Pull Accumulator
      //
      // Pulls an 8bit value from the stack and into the accumulator. The zero
      // and negative flags are set as appropriate.
      cpu->A = cpu_pop_8b(cpu);
      cpu_set_zero_negative_flags(cpu, cpu->A);
      cpu->PC += instruction.bytes;
      break;
    case 0x69:
      /*
       * ADC - Add With Carry (immediate)
       *
       * Adds the contents of a memory location to the accumulator together with
       * the carry bit. If overflow occurs, the carry bit is set.
       */
      {
        const uint8_t value = cpu->ram[cpu->PC + 1];
        const uint16_t result = cpu->A + value + (cpu->P & FLAGS_CARRY);

        const bool sign_a = cpu->A & 0x80;
        const bool sign_value = value & 0x80;
        const bool sign_result = result & 0x80;

        /*
         * TODO(ton): below is not enough!
         */
        BIT_SET_IF(sign_a == sign_value && sign_a != sign_result, cpu->P,
                   FLAGS_BIT_OVERFLOW);
        BIT_SET_IF(result > 0xff, cpu->P, FLAGS_BIT_CARRY);

        cpu->A = (result & 0xff);
        cpu_set_zero_negative_flags(cpu, cpu->A);

        cpu->PC += instruction.bytes;
      }
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
    case 0x78:
      // SEI - Set Interrupt Disable
      //
      // Set the interrupt disable flag to one.
      cpu->P |= FLAGS_INTERRUPT_DISABLE;
      cpu->PC += instruction.bytes;
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
    case 0xA0:
      /*
       * LDY - Load Y Register (immediate)
       *
       * Loads a byte of memory into the Y register, setting the zero and
       * negative flags as appropriate.
       */
      cpu->Y = cpu->ram[cpu->PC + 1];
      cpu_set_zero_negative_flags(cpu, cpu->Y);
      cpu->PC += instruction.bytes;
      break;
    case 0xA2:
      /*
       * LDX - Load X Register (immediate)
       *
       * Loads a byte of memory into the X register, setting the zero and
       * negative flags as appropriate.
       */
      cpu->X = cpu->ram[cpu->PC + 1];
      cpu_set_zero_negative_flags(cpu, cpu->X);
      cpu->PC += instruction.bytes;
      break;
    case 0xA9:
      // LDA - Load Accumulator (immediate)
      //
      // Loads a byte of memory into the accumulator setting the zero and
      // negative flags as appropriate.
      cpu->A = cpu->ram[cpu->PC + 1];
      cpu_set_zero_negative_flags(cpu, cpu->A);
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
    case 0xB8:
      /*
       * CLV - Clear Overflow Flag
       *
       * Clears the overflow flag.
       */
      BIT_CLEAR(cpu->P, FLAGS_BIT_OVERFLOW);
      cpu->PC += instruction.bytes;
      break;
    case 0xC0:
      /*
       * CPY - Compare Y Register
       *
       * Compares the contents of the Y register with another value from memory
       * and sets the zero and carry flags as appropriate. The negative flag is
       * set in case bit 7 of the result of (Y - value) is set.
       */
      {
        const uint8_t value = cpu->ram[cpu->PC + 1];
        BIT_SET_IF(cpu->Y >= value, cpu->P, FLAGS_BIT_CARRY);
        BIT_SET_IF(cpu->Y == value, cpu->P, FLAGS_BIT_ZERO);
        BIT_SET_IF((cpu->Y - value) & 0x80, cpu->P, FLAGS_BIT_NEGATIVE);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xC8:
      /*
       * INY - Increment Y Register
       *
       * Increments the value in the Y register, and sets the zero and negative
       * flags appropriately.
       */
      cpu->Y++;
      cpu_set_zero_negative_flags(cpu, cpu->Y);
      cpu->PC += instruction.bytes;
      break;
    case 0xC9:
      /*
       * CMP - Compare
       *
       * Compares the contents of the accumulator with another value from
       * memory and sets the zero and carry flags as appropriate. The negative
       * flag is set in case bit 7 of the result of (A - value) is set.
       */
      {
        const uint8_t value = cpu->ram[cpu->PC + 1];
        BIT_SET_IF(cpu->A >= value, cpu->P, FLAGS_BIT_CARRY);
        BIT_SET_IF(cpu->A == value, cpu->P, FLAGS_BIT_ZERO);
        BIT_SET_IF((cpu->A - value) & 0x80, cpu->P, FLAGS_BIT_NEGATIVE);
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
    case 0xD8:
      /*
       * CLD - Clear Decimal Mode
       *
       * Sets the decimal mode flag to zero.
       */
      BIT_CLEAR(cpu->P, FLAGS_BIT_DECIMAL);
      cpu->PC += instruction.bytes;
      break;
    case 0xE0:
      /*
       * CPX - Compare X Register
       *
       * Compares the contents of the X register with another value from memory
       * and sets the zero and carry flags as appropriate. The negative flag is
       * set in case bit 7 of the result of (X - value) is set.
       */
      {
        const uint8_t value = cpu->ram[cpu->PC + 1];
        BIT_SET_IF(cpu->X >= value, cpu->P, FLAGS_BIT_CARRY);
        BIT_SET_IF(cpu->X == value, cpu->P, FLAGS_BIT_ZERO);
        BIT_SET_IF((cpu->X - value) & 0x80, cpu->P, FLAGS_BIT_NEGATIVE);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xE9:
      /*
       * SBC - Subtract With Carry (immediate)
       *
       * Subtracts the contents of a memory location from the accumulator
       * together with the not of the carry bit. If overflow occurs, the carry
       * bit is clear.
       */
      {
        const uint8_t value = cpu->ram[cpu->PC + 1];
        const int16_t result = cpu->A - value - (1 - (cpu->P & FLAGS_CARRY));

        const bool sign_a = cpu->A & 0x80;
        const bool sign_value = value & 0x80;
        const bool sign_result = result & 0x80;

        /*
         * TODO(ton): below is not enough!
         */
        BIT_SET_IF(sign_a == sign_value && sign_a != sign_result, cpu->P,
                   FLAGS_BIT_OVERFLOW);
        BIT_SET_IF(cpu->A >= value, cpu->P, FLAGS_BIT_CARRY);

        cpu->A = (result & 0xff);
        cpu_set_zero_negative_flags(cpu, cpu->A);

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
    case 0xF8:
      /*
       * SED - Set Decimal Flag
       *
       * Set the decimal flag to one.
       */
      BIT_SET(cpu->P, FLAGS_BIT_DECIMAL);
      cpu->PC += instruction.bytes;
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
