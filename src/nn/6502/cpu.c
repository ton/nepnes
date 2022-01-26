#include "cpu.h"

#include <nn/6502/instruction.h>
#include <nn/std/util.h>

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
  cpu->S += 2;
  return cpu_read_16b(cpu, STACK_OFFSET + cpu->S - 1);
}

/*
 * Pushes a 16-bit value on to the stack.
 */
static void cpu_push_16b(struct Cpu *cpu, uint16_t i)
{
  cpu_write_16b(cpu, STACK_OFFSET + cpu->S - 1, i);
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
 * Performs an 8-bit add with carry, adding the given value to the accumulator
 * register.
 */
static void cpu_addc(struct Cpu *cpu, uint8_t v)
{
  /* The overflow flag is set in case the signed result of the addition does not
   * fit in the range -128 to 127. In case of signed addition, the operands are
   * encoded in two's complement. In the table below, we consider addition of
   * two two's complement numbers `u` and `v`, where `u7` and `v7` represent the
   * sign bits of `u` and `v` respectively, and `c` represents the carry bit
   * that results from adding the most significant bits of the (signed)
   * representation of `u` and `v`, thus the result of adding bit 6 of both `u`
   * and `v`. `C` is the resulting carry bit, r7 is the sign bit of the result
   * value (the 7-th bit), and `V` is the overflow bit.
   *
   *   u7    v7    c6 | r7   C    V
   *  ----------------+--------------
   *   0     0     0  | 0    0    0    (no unsigned carry, no signed overflow)
   *   0     0     1  | 1    0    1    (no unsigned carry, signed overflow)
   *   0     1     0  | 1    0    0    (no unsigned carry no signed overflow)
   *   0     1     1  | 0    1    0    (unsigned carry, no signed overflow)
   *   1     0     0  | 1    0    0    (no unsigned carry, no signed overflow)
   *   1     0     1  | 0    1    0    (unsigned carry, no signed overflow)
   *   1     1     0  | 0    1    1    (unsigned carry, signed overflow)
   *   1     1     1  | 1    1    0    (unsigned carry, no signed overflow)
   *
   * From this table, it is trivial to derive a formula for the overflow bit:
   *
   *   (!u7 && !v7 && c6) || (u7 && v7 && !c6)
   *
   * or from the result:
   *
   *   (!u7 && !v7 && r7) || (u7 && v7 && !r7)
   *
   * Thus, in case the sign bit of the inputs is the same, but differs from the
   * sign bit of the result, overflow occurs:
   *
   *   (u ^ r) & (v ^ r) & 0x80
   */
  const uint8_t u = cpu->A;
  const uint16_t r = u + v + (cpu->P & FLAGS_CARRY);

  BIT_SET_IF((u ^ r) & (v ^ r) & 0x80, cpu->P, FLAGS_BIT_OVERFLOW);
  BIT_SET_IF(r > 0xff, cpu->P, FLAGS_BIT_CARRY);

  cpu->A = (r & 0xff);
  cpu_set_zero_negative_flags(cpu, cpu->A);
}

/*
 * Reads an 8-bit value at the given address. Specifying an out-of-bounds
 * address results in undefined behavior.
 */
uint8_t cpu_read_8b(struct Cpu *cpu, Address a)
{
  return cpu->ram[a];
}

/*
 * Reads a 16-bit value at the given address. Specifying an out-of-bounds
 * address results in undefined behavior.
 */
uint16_t cpu_read_16b(struct Cpu *cpu, Address a)
{
  return cpu->ram[a] + (cpu->ram[a + 1] << 8);  // little endian
}

/*
 * Reads a 16-bit value at the given address, and then uses that value as an
 * address to read the final 16-bit value. Only used by the JMP instruction.
 */
uint16_t cpu_read_indirect_16b(struct Cpu *cpu, Address a)
{
  if ((a & 0xff) == 0xff)
  {
    /* "An original 6502 does not correctly fetch the target address if the
     * indirect vector falls on a page boundary (e.g. $xxFF where xx is any
     * value from $00 to $FF). In this case it fetches the LSB from $xxFF as
     * expected but takes the MSB from $xx00. This is fixed in some later chips
     * like the 65SC02 so for compatibility always ensure the indirect vector is
     * not at the end of the page"
     *
     *                           - taken from the 6502 Reference; obelisk.me.uk
     */
    const uint8_t lsb = cpu_read_8b(cpu, a);
    const uint8_t msb = cpu_read_8b(cpu, a & 0xff00);
    return lsb + (msb << 8);
  }
  else
  {
    return cpu_read_16b(cpu, a);
  }
}

/*
 * Writes a 16-bit value at the given address. Specifying an out-of-bounds
 * address results in undefined behavior.
 */
void cpu_write_16b(struct Cpu *cpu, Address a, uint16_t x)
{
  cpu->ram[a] = x & 0xff;  // little endian
  cpu->ram[a + 1] = (x >> 8) & 0xff;
}

/*
 * Reads a pre-indexed address stored in the zero table in the range $00-$ff. In
 * case the offset is $ff, the most significant byte of the address is read from
 * $00.
 */
Address cpu_read_indirect_address(struct Cpu *cpu, uint8_t offset)
{
  return cpu_read_8b(cpu, offset) + ((Address)cpu_read_8b(cpu, (uint8_t)(offset + 1)) << 8);
}

/*
 * Reads a pre-indexed address stored in the zero table. The offset in the table
 * is calculated by adding the given offset and the value in the X register,
 * modulo $ff. In case the resulting offset is $ff, the most significant byte of
 * the address is read from $00.
 */
Address cpu_read_indirect_x_address(struct Cpu *cpu, uint8_t offset)
{
  const uint8_t x_offset = offset + cpu->X;
  return cpu_read_8b(cpu, x_offset) + ((Address)cpu_read_8b(cpu, (uint8_t)(x_offset + 1)) << 8);
}

/*
 * Performs a pre-indexed indirect addressing using the address stored in the
 * table in the zero page at the given offset. This performs a memory lookup
 * using the indirect,x addressing mode (AM_INDIRECT_X).
 */
uint8_t cpu_read_indirect_x(struct Cpu *cpu, uint8_t offset)
{
  return cpu_read_8b(cpu, cpu_read_indirect_x_address(cpu, offset));
}

/*
 * Reads a post-indexed address stored in the zero table. The offset in the
 * table is given and contains the least significant byte of the address. This
 * address is subsequently added to the value of the Y-register to get to the
 * final address.
 */
Address cpu_read_indirect_y_address(struct Cpu *cpu, uint8_t offset)
{
  return cpu_read_8b(cpu, offset) + ((Address)cpu_read_8b(cpu, (uint8_t)(offset + 1)) << 8) +
         cpu->Y;
}

/*
 * Performs a post-indexed indirect addressing using the address stored in the
 * table in the zero page at the given offset. This performs a memory lookup
 * using the indirect,y addressing mode (AM_INDIRECT_Y).
 */
uint8_t cpu_read_indirect_y(struct Cpu *cpu, uint8_t offset)
{
  return cpu_read_8b(cpu, cpu_read_indirect_y_address(cpu, offset));
}

/*
 * Reads a zero page offset from the given address in memory, and converts it to
 * an address that can be used for the zero page, X addressing mode
 * (AM_ZERO_PAGE_X).
 */
uint8_t cpu_make_zero_page_x_offset(struct Cpu *cpu, uint8_t offset)
{
  return (uint8_t)(offset + cpu->X);
}

/*
 * Reads a value from the zero page at the given offset, after adding the value
 * in the X-register to the offset, taking the module of 0xFF (AM_ZERO_PAGE_X).
 */
uint8_t cpu_read_zero_page_x(struct Cpu *cpu, uint8_t offset)
{
  return cpu_read_8b(cpu, cpu_make_zero_page_x_offset(cpu, offset));
}

/*
 * Reads a zero page offset from the given address in memory, and converts it to
 * an address that can be used for the zero page, Y addressing mode
 * (AM_ZERO_PAGE_Y).
 */
uint8_t cpu_make_zero_page_y_offset(struct Cpu *cpu, uint8_t offset)
{
  return (uint8_t)(offset + cpu->Y);
}

/*
 * Reads a value from the zero page at the given offset, after adding the value
 * in the Y-register to the offset, taking the module of 0xFF (AM_ZERO_PAGE_Y).
 */
uint8_t cpu_read_zero_page_y(struct Cpu *cpu, uint8_t offset)
{
  return cpu_read_8b(cpu, cpu_make_zero_page_y_offset(cpu, offset));
}

/*
 * Returns 1 in case adding the given offset to the address results in a page
 * cross, returns 0 otherwise. A page runs from $--00 to $--ff.
 */
int cpu_page_cross(Address address, uint8_t offset)
{
  /* TODO(ton): is there a more efficient/elegant way to check this? */
  return 0xff - offset < (address & 0xff);
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
    case 0x01:
      /*
       * ORA - Logical Inclusive OR (indirect, X)
       *
       * An inclusive OR is performed, bit by bit, on the contents of the
       * accumulator and some operand value, and stores the result in the
       * accumulator. Updates the zero and negative flags accordingly.
       */
      cpu->A |= cpu_read_indirect_x(cpu, cpu->ram[cpu->PC + 1]);
      cpu_set_zero_negative_flags(cpu, cpu->A);
      cpu->PC += instruction.bytes;
      break;
    case 0x05:
      /*
       * ORA - Logical Inclusive OR (zero page)
       *
       * An inclusive OR is performed, bit by bit, on the contents of the
       * accumulator and some operand value, and stores the result in the
       * accumulator. Updates the zero and negative flags accordingly.
       */
      cpu->A |= cpu->ram[cpu->ram[cpu->PC + 1]];
      cpu_set_zero_negative_flags(cpu, cpu->A);
      cpu->PC += instruction.bytes;
      break;
    case 0x06:
      /*
       * ASL - Arithmetic Shift Left (zero page)
       *
       * This operation shifts all the bits of the accumulator or memory
       * contents one bit left. Bit 0 is set to 0 and bit 7 is placed in the
       * carry flag. The effect of this operation is to multiply the memory
       * contents by 2 (ignoring 2's complement considerations), setting the
       * carry if the result will not fit in 8 bits. The zero and negative flags
       * are set according to the calculated value.
       */
      {
        uint8_t *value = cpu->ram + cpu->ram[cpu->PC + 1];
        BIT_SET_IF(*value & 0x80, cpu->P, FLAGS_BIT_CARRY);
        *value <<= 1;
        *value &= 0xfe;
        cpu_set_zero_negative_flags(cpu, *value);
        cpu->PC += instruction.bytes;
      }
      break;
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
       * An inclusive OR is performed, bit by bit, on the contents of the
       * accumulator and some operand value, and stores the result in the
       * accumulator. Updates the zero and negative flags accordingly.
       */
      cpu->A |= cpu->ram[cpu->PC + 1];
      cpu_set_zero_negative_flags(cpu, cpu->A);
      cpu->PC += instruction.bytes;
      break;
    case 0x0a:
      /*
       * ASL - Arithmetic Shift Left (accumulator)
       *
       * This operation shifts all the bits of the accumulator or memory
       * contents one bit left. Bit 0 is set to 0 and bit 7 is placed in the
       * carry flag. The effect of this operation is to multiply the memory
       * contents by 2 (ignoring 2's complement considerations), setting the
       * carry if the result will not fit in 8 bits. The zero and negative flags
       * are set according to the calculated value.
       */
      BIT_SET_IF(cpu->A & 0x80, cpu->P, FLAGS_BIT_CARRY);
      cpu->A <<= 1;
      cpu->A &= 0xfe;
      cpu_set_zero_negative_flags(cpu, cpu->A);
      cpu->PC += instruction.bytes;
      break;
    case 0x0d:
      /*
       * ORA - Logical Inclusive OR (absolute)
       *
       * An inclusive OR is performed, bit by bit, on the contents of the
       * accumulator and some operand value, and stores the result in the
       * accumulator. Updates the zero and negative flags accordingly.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        cpu->A |= cpu->ram[address];
        cpu_set_zero_negative_flags(cpu, cpu->A);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x0e:
      /*
       * ASL - Arithmetic Shift Left (absolute)
       *
       * This operation shifts all the bits of the accumulator or memory
       * contents one bit left. Bit 0 is set to 0 and bit 7 is placed in the
       * carry flag. The effect of this operation is to multiply the memory
       * contents by 2 (ignoring 2's complement considerations), setting the
       * carry if the result will not fit in 8 bits. The zero and negative flags
       * are set according to the calculated value.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        uint8_t *value = cpu->ram + address;
        BIT_SET_IF(*value & 0x80, cpu->P, FLAGS_BIT_CARRY);
        *value <<= 1;
        *value &= 0xfe;
        cpu_set_zero_negative_flags(cpu, *value);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x10:
      /*
       * BPL - Branch if Positive
       *
       * If the negative flag is clear then add the relative displacement to the
       * program counter to cause a branch to a new location.
       */
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
    case 0x11:
      /*
       * ORA - Logical Inclusive OR (indirect), Y
       *
       * An inclusive OR is performed, bit by bit, on the contents of the
       * accumulator and some operand value, and stores the result in the
       * accumulator. Updates the zero and negative flags accordingly.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        cpu->A |= cpu_read_indirect_y(cpu, operand);
        cpu->cycle += cpu_page_cross(cpu_read_16b(cpu, operand), cpu->Y);
        cpu_set_zero_negative_flags(cpu, cpu->A);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x15:
      /*
       * ORA - Logical Inclusive OR (zero page, X)
       *
       * An inclusive OR is performed, bit by bit, on the contents of the
       * accumulator and some operand value, and stores the result in the
       * accumulator. Updates the zero and negative flags accordingly.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        cpu->A |= cpu_read_zero_page_x(cpu, operand);
        cpu_set_zero_negative_flags(cpu, cpu->A);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x16:
      /*
       * ASL - Arithmetic Shift Left (zero page, X)
       *
       * This operation shifts all the bits of the accumulator or memory
       * contents one bit left. Bit 0 is set to 0 and bit 7 is placed in the
       * carry flag. The effect of this operation is to multiply the memory
       * contents by 2 (ignoring 2's complement considerations), setting the
       * carry if the result will not fit in 8 bits. The zero and negative flags
       * are set according to the calculated value.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        uint8_t *value = cpu->ram + cpu_make_zero_page_x_offset(cpu, operand);
        BIT_SET_IF(*value & 0x80, cpu->P, FLAGS_BIT_CARRY);
        *value <<= 1;
        *value &= 0xfe;
        cpu_set_zero_negative_flags(cpu, *value);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x18:
      /*
       * CLC - Clear Carry Flag
       *
       * Sets the carry flag to zero.
       */
      cpu->P &= ~FLAGS_CARRY;
      cpu->PC += instruction.bytes;
      break;
    case 0x19:
      /*
       * ORA - Logical Inclusive OR (absolute, Y)
       *
       * An inclusive OR is performed, bit by bit, on the contents of the
       * accumulator and some operand value, and stores the result in the
       * accumulator. Updates the zero and negative flags accordingly.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        cpu->A |= cpu_read_8b(cpu, address + cpu->Y);
        cpu_set_zero_negative_flags(cpu, cpu->A);
        cpu->cycle += cpu_page_cross(address, cpu->Y);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x20:
      /*
       * JSR - Jump to Subroutine
       *
       * The JSR instruction pushes the address (minus one) of the return point
       * on to the stack and then sets the program counter to the target memory
       * address.
       */

      /* Push next instruction address (-1) onto the stack. */
      cpu_push_16b(cpu, cpu->PC + instruction.bytes - 1);
      cpu->PC = cpu_read_16b(cpu, cpu->PC + 1);
      break;
    case 0x21:
      /*
       * AND - Logical And (indirect, X)
       *
       * A logical AND is performed, bit by bit, on the accumulator
       * contents using the contents of a byte of memory.
       */
      cpu->A &= cpu_read_indirect_x(cpu, cpu->ram[cpu->PC + 1]);
      cpu_set_zero_negative_flags(cpu, cpu->A);
      cpu->PC += instruction.bytes;
      break;
    case 0x24:
      /*
       * BIT - Bit Test (zero page)
       *
       * This instructions is used to test if one or more bits are set in a
       * target memory location. The mask pattern in A is AND'ed with the value
       * in memory to set or clear the zero flag, but the result is not kept.
       * Bits 7 and 6 of the value from memory are copied into the N and V
       * flags.
       */
      {
        const uint8_t address = cpu->ram[cpu->PC + 1];
        const uint8_t value = cpu->ram[address];
        BIT_SET_IF(!(cpu->A & value), cpu->P, FLAGS_BIT_ZERO);
        cpu->P = (cpu->P & 0x3f) | (value & 0xc0);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x25:
      /*
       * AND - Logical And (zero page)
       *
       * A logical AND is performed, bit by bit, on the accumulator
       * contents using the contents of a byte of memory.
       */
      cpu->A &= cpu->ram[cpu->ram[cpu->PC + 1]];
      cpu_set_zero_negative_flags(cpu, cpu->A);
      cpu->PC += instruction.bytes;
      break;
    case 0x26:
      /*
       * ROL - Rotate Left (zero page)
       *
       * Move each of the bits of the value at the given location in the zero
       * page one place to the left. Bit 0 is filled with the current value of
       * the carry flag whilst the old bit 7 becomes the new carry flag value.
       * The zero and negative flags are set as appropriate.
       */
      {
        uint8_t *value = cpu->ram + cpu->ram[cpu->PC + 1];
        const uint8_t new_carry = *value & 0x80;
        *value <<= 1;
        BIT_SET_IF(cpu->P & FLAGS_CARRY, *value, 0);
        BIT_SET_IF(new_carry, cpu->P, FLAGS_BIT_CARRY);
        cpu_set_zero_negative_flags(cpu, *value);
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
    case 0x2a:
      /*
       * ROL - Rotate Left (accumulator)
       *
       * Move each of the bits in A one place to the left. Bit 0 is filled with
       * the current value of the carry flag whilst the old bit 7 becomes the
       * new carry flag value. The zero and negative flags are set as
       * appropriate.
       */
      {
        const uint8_t new_carry = cpu->A & 0x80;
        cpu->A <<= 1;
        BIT_SET_IF(cpu->P & FLAGS_CARRY, cpu->A, 0);
        BIT_SET_IF(new_carry, cpu->P, FLAGS_BIT_CARRY);
        cpu_set_zero_negative_flags(cpu, cpu->A);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x2c:
      /*
       * BIT - Bit Test (absolute)
       *
       * This instructions is used to test if one or more bits are set in a
       * target memory location. The mask pattern in A is AND'ed with the value
       * in memory to set or clear the zero flag, but the result is not kept.
       * Bits 7 and 6 of the value from memory are copied into the N and V
       * flags.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        const uint8_t value = cpu->ram[address];
        BIT_SET_IF(!(cpu->A & value), cpu->P, FLAGS_BIT_ZERO);
        cpu->P = (cpu->P & 0x3f) | (value & 0xc0);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x2d:
      /*
       * AND - Logical And (absolute)
       *
       * A logical AND is performed, bit by bit, on the accumulator
       * contents using the contents of a byte of memory.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        cpu->A &= cpu->ram[address];
        cpu_set_zero_negative_flags(cpu, cpu->A);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x2e:
      /*
       * ROL - Rotate Left (absolute)
       *
       * Move each of the bits of the value at the given location in the zero
       * page one place to the left. Bit 0 is filled with the current value of
       * the carry flag whilst the old bit 7 becomes the new carry flag value.
       * The zero and negative flags are set as appropriate.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        uint8_t *value = cpu->ram + address;
        const uint8_t new_carry = *value & 0x80;
        *value <<= 1;
        BIT_SET_IF(cpu->P & FLAGS_CARRY, *value, 0);
        BIT_SET_IF(new_carry, cpu->P, FLAGS_BIT_CARRY);
        cpu_set_zero_negative_flags(cpu, *value);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x30:
      /*
       * BMI - Branch If Minus (relative)
       *
       * If the negative flag is set, then add the relative displacement to the
       * program counter to cause a branch to a new location.
       */
      if (cpu->P & FLAGS_NEGATIVE)
      {
        cpu->PC += instruction.bytes + cpu_read_8b(cpu, cpu->PC + 1);
        cpu->cycle++;
      }
      else
      {
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x31:
      /*
       * AND - Logical And (indirect), Y
       *
       * A logical AND is performed, bit by bit, on the accumulator
       * contents using the contents of a byte of memory.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        cpu->A &= cpu_read_indirect_y(cpu, operand);
        cpu->cycle += cpu_page_cross(cpu_read_16b(cpu, operand), cpu->Y);
        cpu_set_zero_negative_flags(cpu, cpu->A);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x35:
      /*
       * AND - Logical And (zero page, X)
       *
       * A logical AND is performed, bit by bit, on the accumulator
       * contents using the contents of a byte of memory.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        cpu->A &= cpu_read_zero_page_x(cpu, operand);
        cpu_set_zero_negative_flags(cpu, cpu->A);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x36:
      /*
       * ROL - Rotate Left (zero page, X)
       *
       * Move each of the bits of the value at the given location in the zero
       * page one place to the left. Bit 0 is filled with the current value of
       * the carry flag whilst the old bit 7 becomes the new carry flag value.
       * The zero and negative flags are set as appropriate.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        uint8_t *value = cpu->ram + cpu_make_zero_page_x_offset(cpu, operand);
        const uint8_t new_carry = *value & 0x80;
        *value <<= 1;
        BIT_SET_IF(cpu->P & FLAGS_CARRY, *value, 0);
        BIT_SET_IF(new_carry, cpu->P, FLAGS_BIT_CARRY);
        cpu_set_zero_negative_flags(cpu, *value);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x38:
      /*
       * SEC - Set Carry Flag
       *
       * Sets the carry flag to one.
       */
      cpu->P |= FLAGS_CARRY;
      cpu->PC += instruction.bytes;
      break;
    case 0x39:
      /*
       * AND - Logical And (absolute, Y)
       *
       * A logical AND is performed, bit by bit, on the accumulator
       * contents using the contents of a byte of memory.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        cpu->A &= cpu_read_8b(cpu, address + cpu->Y);
        cpu_set_zero_negative_flags(cpu, cpu->A);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x40:
      /*
       * RTI - Return from Interrupt
       *
       * The RTI instruction is used at the end of an interrupt processing
       * routine. It pulls the processor flags from the stack followed by the
       * program counter. When popping the CPU status flags, bits 4 and 5, the
       * 'B-flag' is ignored.
       */
      cpu->P = (cpu_pop_8b(cpu) & 0xcf) | (cpu->P & 0x30);
      cpu->PC = cpu_pop_16b(cpu);
      break;
    case 0x41:
      /*
       * EOR - Exclusive OR (indirect, X)
       *
       * Performs an exclusive OR, bit by bit, on the value in the accumulator
       * and the given operand, and stores the result in the accumulator. The
       * zero and negative flags are set accordingly.
       */
      cpu->A ^= cpu_read_indirect_x(cpu, cpu->ram[cpu->PC + 1]);
      cpu_set_zero_negative_flags(cpu, cpu->A);
      cpu->PC += instruction.bytes;
      break;
    case 0x45:
      /*
       * EOR - Exclusive OR (zero page)
       *
       * Performs an exclusive OR, bit by bit, on the value in the accumulator
       * and the given operand, and stores the result in the accumulator. The
       * zero and negative flags are set accordingly.
       */
      cpu->A ^= cpu->ram[cpu->ram[cpu->PC + 1]];
      cpu_set_zero_negative_flags(cpu, cpu->A);
      cpu->PC += instruction.bytes;
      break;
    case 0x46:
      /*
       * LSR - Logical Shift Right (zero page)
       *
       * Each of the bits of the value in the zero page at the given location is
       * shifted one place to the right. The bit that was in bit 0 is shifted
       * into the carry flag. Bit 7 is set to zero. The zero and negative flags
       * are set according to the calculated value.
       */
      {
        uint8_t *value = cpu->ram + cpu->ram[cpu->PC + 1];
        BIT_SET_IF(*value & 0x01, cpu->P, FLAGS_BIT_CARRY);
        *value = (*value >> 1) & 0x7f;
        cpu_set_zero_negative_flags(cpu, *value);
        cpu->PC += instruction.bytes;
      }
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
    case 0x4a:
      /*
       * LSR - Logical Shift Right (accumulator)
       *
       * Each of the bits in A is shifted one place to the right. The bit that
       * was in bit 0 is shifted into the carry flag. Bit 7 is set to zero. The
       * zero and negative flags are set according to the calculated value.
       */
      BIT_SET_IF(cpu->A & 0x01, cpu->P, FLAGS_BIT_CARRY);
      cpu->A = (cpu->A >> 1) & 0x7f;
      cpu_set_zero_negative_flags(cpu, cpu->A);
      cpu->PC += instruction.bytes;
      break;
    case 0x4c:
      /*
       * JMP - Jump (absolute)
       *
       * Sets the program counter to the address specified by the operand.
       */
      cpu->PC = cpu_read_16b(cpu, cpu->PC + 1);
      break;
    case 0x4d:
      /*
       * EOR - Exclusive OR (absolute)
       *
       * Performs an exclusive OR, bit by bit, on the value in the accumulator
       * and the given operand, and stores the result in the accumulator. The
       * zero and negative flags are set accordingly.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        cpu->A ^= cpu->ram[address];
        cpu_set_zero_negative_flags(cpu, cpu->A);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x4e:
      /*
       * LSR - Logical Shift Right (absolute)
       *
       * Each of the bits of the value in the zero page at the given location is
       * shifted one place to the right. The bit that was in bit 0 is shifted
       * into the carry flag. Bit 7 is set to zero. The zero and negative flags
       * are set according to the calculated value.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        uint8_t *value = cpu->ram + address;
        BIT_SET_IF(*value & 0x01, cpu->P, FLAGS_BIT_CARRY);
        *value = (*value >> 1) & 0x7f;
        cpu_set_zero_negative_flags(cpu, *value);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x50:
      /*
       * BVC - Branch if Overflow Clear
       *
       * If the overflow flag is clear then add the relative displacement to the
       * program counter to cause a branch to a new location.
       */
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
    case 0x51:
      /*
       * EOR - Exclusive OR (indirect), Y
       *
       * Performs an exclusive OR, bit by bit, on the value in the accumulator
       * and the given operand, and stores the result in the accumulator. The
       * zero and negative flags are set accordingly.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        cpu->A ^= cpu_read_indirect_y(cpu, operand);
        cpu->cycle += cpu_page_cross(cpu_read_16b(cpu, operand), cpu->Y);
        cpu_set_zero_negative_flags(cpu, cpu->A);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x55:
      /*
       * EOR - Exclusive OR (zero page, X)
       *
       * Performs an exclusive OR, bit by bit, on the value in the accumulator
       * and the given operand, and stores the result in the accumulator. The
       * zero and negative flags are set accordingly.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        cpu->A ^= cpu_read_zero_page_x(cpu, operand);
        cpu_set_zero_negative_flags(cpu, cpu->A);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x56:
      /*
       * LSR - Logical Shift Right (zero page, X)
       *
       * Each of the bits of the value in the zero page at the given location is
       * shifted one place to the right. The bit that was in bit 0 is shifted
       * into the carry flag. Bit 7 is set to zero. The zero and negative flags
       * are set according to the calculated value.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        uint8_t *value = cpu->ram + cpu_make_zero_page_x_offset(cpu, operand);
        BIT_SET_IF(*value & 0x01, cpu->P, FLAGS_BIT_CARRY);
        *value = (*value >> 1) & 0x7f;
        cpu_set_zero_negative_flags(cpu, *value);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x59:
      /*
       * EOR - Exclusive OR (absolute, Y)
       *
       * Performs an exclusive OR, bit by bit, on the value in the accumulator
       * and the given operand, and stores the result in the accumulator. The
       * zero and negative flags are set accordingly.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        cpu->A ^= cpu_read_8b(cpu, address + cpu->Y);
        cpu_set_zero_negative_flags(cpu, cpu->A);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x60:
      /*
       * RTS - Return from Subroutine
       *
       * The RTS instruction is used at the end of a subroutine to return to the
       * calling routine. It pulls the program counter (minus one) from the
       * stack.
       */
      cpu->PC = cpu_pop_16b(cpu);
      cpu->PC += instruction.bytes;
      break;
    case 0x61:
      /*
       * ADC - Add With Carry (indirect, X)
       *
       * Adds the contents of a memory location to the accumulator together with
       * the carry bit. If overflow occurs, the carry bit is set.
       */
      cpu_addc(cpu, cpu_read_indirect_x(cpu, cpu->ram[cpu->PC + 1]));
      cpu->PC += instruction.bytes;
      break;
    case 0x65:
      /*
       * ADC - Add With Carry (zero page)
       *
       * Adds the contents of a memory location to the accumulator together with
       * the carry bit. If overflow occurs, the carry bit is set.
       */
      cpu_addc(cpu, cpu->ram[cpu->ram[cpu->PC + 1]]);
      cpu->PC += instruction.bytes;
      break;
    case 0x68:
      /*
       * PLA - Pull Accumulator
       *
       * Pulls an 8bit value from the stack and into the accumulator. The zero
       * and negative flags are set as appropriate.
       */
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
      cpu_addc(cpu, cpu->ram[cpu->PC + 1]);
      cpu->PC += instruction.bytes;
      break;
    case 0x66:
      /*
       * ROR - Rotate Right (zero page)
       *
       * Move each of the bits of the value at the given location in the zero
       * page one place to the right. Bit 7 is filled with the current value of
       * the carry flag whilst the old bit 0 becomes the new carry flag value.
       * The zero and negative flags are set as appropriate.
       */
      {
        uint8_t *value = cpu->ram + cpu->ram[cpu->PC + 1];
        const uint8_t new_carry = *value & 0x01;
        *value >>= 1;
        BIT_SET_IF(cpu->P & FLAGS_CARRY, *value, 7);
        BIT_SET_IF(new_carry, cpu->P, FLAGS_BIT_CARRY);
        cpu_set_zero_negative_flags(cpu, *value);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x6a:
      /*
       * ROR - Rotate Right (accumulator)
       *
       * Move each of the bits in A one place to the right. Bit 7 is filled with
       * the current value of the carry flag whilst the old bit 0 becomes the
       * new carry flag value. The zero and negative flags are set as
       * appropriate.
       */
      {
        const uint8_t new_carry = cpu->A & 0x01;
        cpu->A >>= 1;
        BIT_SET_IF(cpu->P & FLAGS_CARRY, cpu->A, 7);
        BIT_SET_IF(new_carry, cpu->P, FLAGS_BIT_CARRY);
        cpu_set_zero_negative_flags(cpu, cpu->A);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x6c:
      /*
       * JMP - Jump (indirect)
       *
       * Sets the program counter to the address stored at the address specified
       * by the operand.
       */
      cpu->PC = cpu_read_indirect_16b(cpu, cpu_read_16b(cpu, cpu->PC + 1));
      break;
    case 0x6d:
      /*
       * ADC - Add With Carry (absolute)
       *
       * Adds the contents of a memory location to the accumulator together with
       * the carry bit. If overflow occurs, the carry bit is set.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        cpu_addc(cpu, cpu->ram[address]);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x6e:
      /*
       * ROR - Rotate Right (absolute)
       *
       * Move each of the bits of the value at the given location in the zero
       * page one place to the right. Bit 7 is filled with the current value of
       * the carry flag whilst the old bit 0 becomes the new carry flag value.
       * The zero and negative flags are set as appropriate.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        uint8_t *value = cpu->ram + address;
        const uint8_t new_carry = *value & 0x01;
        *value >>= 1;
        BIT_SET_IF(cpu->P & FLAGS_CARRY, *value, 7);
        BIT_SET_IF(new_carry, cpu->P, FLAGS_BIT_CARRY);
        cpu_set_zero_negative_flags(cpu, *value);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x70:
      /*
       * BVS - Branch if Overflow Set
       *
       * If the overflow flag is set then add the relative displacement to the
       * program counter to cause a branch to a new location.
       */
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
    case 0x71:
      /*
       * ADC - Add With Carry (indirect), Y
       *
       * Adds the contents of a memory location to the accumulator together with
       * the carry bit. If overflow occurs, the carry bit is set.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        cpu_addc(cpu, cpu_read_indirect_y(cpu, operand));
        cpu->cycle += cpu_page_cross(cpu_read_16b(cpu, operand), cpu->Y);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x75:
      /*
       * ADC - Add With Carry (zero page, X)
       *
       * Adds the contents of a memory location to the accumulator together with
       * the carry bit. If overflow occurs, the carry bit is set.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        cpu_addc(cpu, cpu_read_zero_page_x(cpu, operand));
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x76:
      /*
       * ROR - Rotate Right (zero page, X)
       *
       * Move each of the bits of the value at the given location in the zero
       * page one place to the right. Bit 7 is filled with the current value of
       * the carry flag whilst the old bit 0 becomes the new carry flag value.
       * The zero and negative flags are set as appropriate.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        uint8_t *value = cpu->ram + cpu_make_zero_page_x_offset(cpu, operand);
        const uint8_t new_carry = *value & 0x01;
        *value >>= 1;
        BIT_SET_IF(cpu->P & FLAGS_CARRY, *value, 7);
        BIT_SET_IF(new_carry, cpu->P, FLAGS_BIT_CARRY);
        cpu_set_zero_negative_flags(cpu, *value);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x78:
      /*
       * SEI - Set Interrupt Disable
       *
       * Set the interrupt disable flag to one.
       */
      cpu->P |= FLAGS_INTERRUPT_DISABLE;
      cpu->PC += instruction.bytes;
      break;
    case 0x79:
      /*
       * ADC - Add With Carry (absolute, Y)
       *
       * Adds the contents of a memory location to the accumulator together with
       * the carry bit. If overflow occurs, the carry bit is set.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        cpu_addc(cpu, cpu_read_8b(cpu, address + cpu->Y));
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x81:
      /*
       * STA - Store Accumulator (indirect, X)
       *
       * Stores the contents of the accumulator into memory.
       */
      {
        const Address address = cpu_read_indirect_x_address(cpu, cpu->ram[cpu->PC + 1]);
        cpu->ram[address] = cpu->A;
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x84:
      /*
       * STY - Store Y Register (zero page)
       *
       * Stores the contents of the Y register into memory.
       */
      {
        uint8_t address = cpu->ram[cpu->PC + 1];
        cpu->ram[address] = cpu->Y;
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x85:
      /*
       * STA - Store Accumulator (zero page)
       *
       * Stores the contents of the accumulator into memory.
       */
      {
        uint8_t address = cpu->ram[cpu->PC + 1];
        cpu->ram[address] = cpu->A;
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x86:
      /*
       * STX - Store X Register (zero page)
       *
       * Stores the contents of the X register into memory.
       */
      {
        uint8_t address = cpu->ram[cpu->PC + 1];
        cpu->ram[address] = cpu->X;
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x88:
      /*
       * DEY - Decrement Y Register
       *
       * Decrements the value in the Y register, and sets the zero and negative
       * flags appropriately.
       */
      cpu->Y--;
      cpu_set_zero_negative_flags(cpu, cpu->Y);
      cpu->PC += instruction.bytes;
      break;
    case 0x8a:
      /*
       * TXA - Transfer X to Accumulator
       *
       * Copies the current value of the X register to the accumulator, and sets
       * the zero and negative flags appropriately.
       */
      cpu->A = cpu->X;
      cpu_set_zero_negative_flags(cpu, cpu->A);
      cpu->PC += instruction.bytes;
      break;
    case 0x8c:
      /*
       * STY - Store Y Register (absolute)
       *
       * Stores the contents of the Y register into memory.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        cpu->ram[address] = cpu->Y;
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x8d:
      /*
       * STA - Store Accumulator (absolute)
       *
       * Stores the contents of the accumulator into memory.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        cpu->ram[address] = cpu->A;
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x8e:
      /*
       * STX - Store X Register (absolute)
       *
       * Stores the contents of the X register into memory.
       */
      {
        const uint16_t address = cpu_read_16b(cpu, cpu->PC + 1);
        cpu->ram[address] = cpu->X;
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x90:
      /*
       * BCC - Branch if Carry Clear
       *
       * If the carry flag is clear then add the relative displacement to the
       * program counter to cause a branch to a new location.
       */
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
    case 0x91:
      /*
       * STA - Store Accumulator (indirect), Y
       *
       * Stores the contents of the accumulator into memory.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        const Address address = cpu_read_indirect_y_address(cpu, operand);
        cpu->ram[address] = cpu->A;
        cpu->PC += instruction.bytes;
        cpu->cycle += cpu_page_cross(cpu_read_16b(cpu, operand), cpu->Y);
      }
      break;
    case 0x94:
      /*
       * STY - Store Y Register (zero page, X)
       *
       * Stores the contents of the Y register into memory.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        const uint8_t address = cpu_make_zero_page_x_offset(cpu, operand);
        cpu->ram[address] = cpu->Y;
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x95:
      /*
       * STA - Store Accumulator (zero page, X)
       *
       * Stores the contents of the accumulator into memory.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        const uint8_t zero_page_offset = cpu_make_zero_page_x_offset(cpu, operand);
        cpu->ram[zero_page_offset] = cpu->A;
        cpu->PC += instruction.bytes;
        cpu->cycle += cpu_page_cross(cpu_read_16b(cpu, operand), cpu->Y);
      }
      break;
    case 0x96:
      /*
       * STX - Store X Register (zero page, Y)
       *
       * Stores the contents of the X register into memory.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        const uint8_t zero_page_y_offset = cpu_make_zero_page_y_offset(cpu, operand);
        cpu->ram[zero_page_y_offset] = cpu->X;
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x98:
      /*
       * TYA - Transfer Y to Accumulator
       *
       * Copies the current value of the Y register to the accumulator, and sets
       * the zero and negative flags appropriately.
       */
      cpu->A = cpu->Y;
      cpu_set_zero_negative_flags(cpu, cpu->A);
      cpu->PC += instruction.bytes;
      break;
    case 0x99:
      /*
       * STA - Store Accumulator (absolute, Y)
       *
       * Stores the contents of the accumulator into memory.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        cpu->ram[address + cpu->Y] = cpu->A;
        cpu->PC += instruction.bytes;
      }
      break;
    case 0x9a:
      /*
       * TXS - Transfer X to Stack Pointer
       *
       * Copies the current value of the X register to the stack pointer (S
       * register).
       */
      cpu->S = cpu->X;
      cpu->PC += instruction.bytes;
      break;
    case 0xa0:
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
    case 0xa1:
      /*
       * LDA - Load Accumulator (indirect, X)
       *
       * Loads a byte of memory into the accumulator setting the zero and
       * negative flags as appropriate.
       */
      {
        cpu->A = cpu_read_indirect_x(cpu, cpu_read_8b(cpu, cpu->PC + 1));
        cpu_set_zero_negative_flags(cpu, cpu->A);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xa2:
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
    case 0xa4:
      /*
       * LDY - Load Y Register (zero page)
       *
       * Loads a byte of memory into the Y register, setting the zero and
       * negative flags as appropriate.
       */
      cpu->Y = cpu->ram[cpu->ram[cpu->PC + 1]];
      cpu_set_zero_negative_flags(cpu, cpu->Y);
      cpu->PC += instruction.bytes;
      break;
    case 0xa5:
      /*
       * LDA - Load Accumulator (zero page)
       *
       * Loads a byte of memory into the accumulator setting the zero and
       * negative flags as appropriate.
       */
      cpu->A = cpu->ram[cpu->ram[cpu->PC + 1]];
      cpu_set_zero_negative_flags(cpu, cpu->A);
      cpu->PC += instruction.bytes;
      break;
    case 0xa6:
      /*
       * LDX - Load X Register (zero page)
       *
       * Loads a byte of memory into the X register, setting the zero and
       * negative flags as appropriate.
       */
      cpu->X = cpu->ram[cpu->ram[cpu->PC + 1]];
      cpu_set_zero_negative_flags(cpu, cpu->X);
      cpu->PC += instruction.bytes;
      break;
    case 0xa8:
      /*
       * TAY - Transfer Accumulator to Y
       *
       * Copies the current value of the accumulator to the Y register, and sets
       * the zero and negative flags appropriately.
       */
      cpu->Y = cpu->A;
      cpu_set_zero_negative_flags(cpu, cpu->Y);
      cpu->PC += instruction.bytes;
      break;
    case 0xa9:
      /*
       * LDA - Load Accumulator (immediate)
       *
       * Loads a byte of memory into the accumulator setting the zero and
       * negative flags as appropriate.
       */
      cpu->A = cpu->ram[cpu->PC + 1];
      cpu_set_zero_negative_flags(cpu, cpu->A);
      cpu->PC += instruction.bytes;
      break;
    case 0xaa:
      /*
       * TAX - Transfer Accumulator to X
       *
       * Copies the current value of the accumulator to the X register, and sets
       * the zero and negative flags appropriately.
       */
      cpu->X = cpu->A;
      cpu_set_zero_negative_flags(cpu, cpu->X);
      cpu->PC += instruction.bytes;
      break;
    case 0xac:
      /*
       * LDY - Load Y Register (absolute)
       *
       * Loads a byte of memory into the Y register, setting the zero and
       * negative flags as appropriate.
       */
      {
        const uint16_t address = cpu_read_16b(cpu, cpu->PC + 1);
        cpu->Y = cpu->ram[address];
        cpu_set_zero_negative_flags(cpu, cpu->Y);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xad:
      /*
       * LDA - Load Accumulator (absolute)
       *
       * Loads a byte of memory into the accumulator setting the zero and
       * negative flags as appropriate.
       */
      {
        const uint16_t address = cpu_read_16b(cpu, cpu->PC + 1);
        cpu->A = cpu->ram[address];
        cpu_set_zero_negative_flags(cpu, cpu->A);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xae:
      /*
       * LDX - Load X Register (absolute)
       *
       * Loads a byte of memory into the X register, setting the zero and
       * negative flags as appropriate.
       */
      {
        const uint16_t address = cpu_read_16b(cpu, cpu->PC + 1);
        cpu->X = cpu->ram[address];
        cpu_set_zero_negative_flags(cpu, cpu->X);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xb0:
      /*
       * BCS - Branch if Carry Set (relative)
       *
       * If the carry flag is set then add the relative displacement to the
       * program counter to cause a branch to a new location.
       */
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
    case 0xb1:
      /*
       * LDA - Load Accumulator (indirect), Y
       *
       * Loads a byte of memory into the accumulator setting the zero and
       * negative flags as appropriate.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        cpu->A = cpu_read_indirect_y(cpu, operand);
        cpu->cycle += cpu_page_cross(cpu_read_16b(cpu, operand), cpu->Y);
        cpu_set_zero_negative_flags(cpu, cpu->A);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xb5:
      /*
       * LDA - Load Accumulator (zero page, X)
       *
       * Loads a byte of memory into the accumulator setting the zero and
       * negative flags as appropriate.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        cpu->A = cpu_read_zero_page_x(cpu, operand);
        cpu_set_zero_negative_flags(cpu, cpu->A);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xb4:
      /*
       * LDY - Load Y Register (zero page, X)
       *
       * Loads a byte of memory into the Y register, setting the zero and
       * negative flags as appropriate.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        cpu->Y = cpu_read_zero_page_x(cpu, operand);
        cpu_set_zero_negative_flags(cpu, cpu->Y);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xb6:
      /*
       * LDX - Load X Register (zero page, Y)
       *
       * Loads a byte of memory into the X register, setting the zero and
       * negative flags as appropriate.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        cpu->X = cpu_read_8b(cpu, cpu_make_zero_page_y_offset(cpu, operand));
        cpu_set_zero_negative_flags(cpu, cpu->X);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xb8:
      /*
       * CLV - Clear Overflow Flag
       *
       * Clears the overflow flag.
       */
      BIT_CLEAR(cpu->P, FLAGS_BIT_OVERFLOW);
      cpu->PC += instruction.bytes;
      break;
    case 0xb9:
      /*
       * LDA - Load Accumulator (absolute, Y)
       *
       * Loads a byte of memory into the accumulator setting the zero and
       * negative flags as appropriate.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        cpu->A = cpu_read_8b(cpu, address + cpu->Y);
        cpu_set_zero_negative_flags(cpu, cpu->A);
        cpu->cycle += cpu_page_cross(address, cpu->Y);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xba:
      /*
       * TSX - Transfer Stack Pointer to X
       *
       * Copies the current value of the stack pointer (S register) to the X
       * register, and sets the zero and negative flags appropriately.
       */
      cpu->X = cpu->S;
      cpu_set_zero_negative_flags(cpu, cpu->X);
      cpu->PC += instruction.bytes;
      break;
    case 0xc0:
      /*
       * CPY - Compare Y Register (immediate)
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
    case 0xc4:
      /*
       * CPY - Compare Y Register (zero page)
       *
       * Compares the contents of the Y register with another value from memory
       * and sets the zero and carry flags as appropriate. The negative flag is
       * set in case bit 7 of the result of (Y - value) is set.
       */
      {
        const uint8_t value = cpu->ram[cpu->ram[cpu->PC + 1]];
        BIT_SET_IF(cpu->Y >= value, cpu->P, FLAGS_BIT_CARRY);
        BIT_SET_IF(cpu->Y == value, cpu->P, FLAGS_BIT_ZERO);
        BIT_SET_IF((cpu->Y - value) & 0x80, cpu->P, FLAGS_BIT_NEGATIVE);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xc1:
      /*
       * CMP - Compare (indirect, X)
       *
       * Compares the contents of the accumulator with another value from
       * memory and sets the zero and carry flags as appropriate. The negative
       * flag is set in case bit 7 of the result of (A - value) is set.
       */
      {
        const uint8_t value = cpu_read_indirect_x(cpu, cpu->ram[cpu->PC + 1]);
        BIT_SET_IF(cpu->A >= value, cpu->P, FLAGS_BIT_CARRY);
        BIT_SET_IF(cpu->A == value, cpu->P, FLAGS_BIT_ZERO);
        BIT_SET_IF((cpu->A - value) & 0x80, cpu->P, FLAGS_BIT_NEGATIVE);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xc5:
      /*
       * CMP - Compare (zero page)
       *
       * Compares the contents of the accumulator with another value from
       * memory and sets the zero and carry flags as appropriate. The negative
       * flag is set in case bit 7 of the result of (A - value) is set.
       */
      {
        const uint8_t value = cpu->ram[cpu->ram[cpu->PC + 1]];
        BIT_SET_IF(cpu->A >= value, cpu->P, FLAGS_BIT_CARRY);
        BIT_SET_IF(cpu->A == value, cpu->P, FLAGS_BIT_ZERO);
        BIT_SET_IF((cpu->A - value) & 0x80, cpu->P, FLAGS_BIT_NEGATIVE);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xc6:
      /*
       * DEC - Decrement memory (zero page)
       *
       * Subtracts one from the value held at a specified memory location
       * setting the zero and negative flags as appropriate.
       */
      {
        uint8_t *value = cpu->ram + cpu->ram[cpu->PC + 1];
        (*value)--;
        cpu_set_zero_negative_flags(cpu, *value);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xc8:
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
    case 0xc9:
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
    case 0xca:
      /*
       * DEX - Decrement X Register
       *
       * Decrements the value in the X register, and sets the zero and negative
       * flags appropriately.
       */
      cpu->X--;
      cpu_set_zero_negative_flags(cpu, cpu->X);
      cpu->PC += instruction.bytes;
      break;
    case 0xcc:
      /*
       * CPY - Compare Y Register (absolute)
       *
       * Compares the contents of the Y register with another value from memory
       * and sets the zero and carry flags as appropriate. The negative flag is
       * set in case bit 7 of the result of (Y - value) is set.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        const uint8_t value = cpu->ram[address];
        BIT_SET_IF(cpu->Y >= value, cpu->P, FLAGS_BIT_CARRY);
        BIT_SET_IF(cpu->Y == value, cpu->P, FLAGS_BIT_ZERO);
        BIT_SET_IF((cpu->Y - value) & 0x80, cpu->P, FLAGS_BIT_NEGATIVE);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xcd:
      /*
       * CMP - Compare (absolute)
       *
       * Compares the contents of the accumulator with another value from
       * memory and sets the zero and carry flags as appropriate. The negative
       * flag is set in case bit 7 of the result of (A - value) is set.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        const uint8_t value = cpu->ram[address];
        BIT_SET_IF(cpu->A >= value, cpu->P, FLAGS_BIT_CARRY);
        BIT_SET_IF(cpu->A == value, cpu->P, FLAGS_BIT_ZERO);
        BIT_SET_IF((cpu->A - value) & 0x80, cpu->P, FLAGS_BIT_NEGATIVE);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xce:
      /*
       * DEC - Decrement memory (absolute)
       *
       * Subtracts one from the value held at a specified memory location
       * setting the zero and negative flags as appropriate.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        uint8_t *value = cpu->ram + address;
        (*value)--;
        cpu_set_zero_negative_flags(cpu, *value);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xd0:
      /*
       * BNE - Branch if Not Equal (relative)
       *
       * If the zero flag is clear then add the relative displacement to the
       * program counter to cause a branch to a new location.
       */
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
    case 0xd1:
      /*
       * CMP - Compare (indirect), Y
       *
       * Compares the contents of the accumulator with another value from
       * memory and sets the zero and carry flags as appropriate. The negative
       * flag is set in case bit 7 of the result of (A - value) is set.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        const uint8_t value = cpu_read_indirect_y(cpu, operand);
        BIT_SET_IF(cpu->A >= value, cpu->P, FLAGS_BIT_CARRY);
        BIT_SET_IF(cpu->A == value, cpu->P, FLAGS_BIT_ZERO);
        BIT_SET_IF((cpu->A - value) & 0x80, cpu->P, FLAGS_BIT_NEGATIVE);
        cpu->cycle += cpu_page_cross(cpu_read_16b(cpu, operand), cpu->Y);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xd5:
      /*
       * CMP - Compare (zero page, X)
       *
       * Compares the contents of the accumulator with another value from
       * memory and sets the zero and carry flags as appropriate. The negative
       * flag is set in case bit 7 of the result of (A - value) is set.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        const uint8_t value = cpu_read_zero_page_x(cpu, operand);
        BIT_SET_IF(cpu->A >= value, cpu->P, FLAGS_BIT_CARRY);
        BIT_SET_IF(cpu->A == value, cpu->P, FLAGS_BIT_ZERO);
        BIT_SET_IF((cpu->A - value) & 0x80, cpu->P, FLAGS_BIT_NEGATIVE);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xd6:
      /*
       * DEC - Decrement memory (zero page, X)
       *
       * Subtracts one from the value held at a specified memory location
       * setting the zero and negative flags as appropriate.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        uint8_t *value = cpu->ram + cpu_make_zero_page_x_offset(cpu, operand);
        (*value)--;
        cpu_set_zero_negative_flags(cpu, *value);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xd8:
      /*
       * CLD - Clear Decimal Mode
       *
       * Sets the decimal mode flag to zero.
       */
      BIT_CLEAR(cpu->P, FLAGS_BIT_DECIMAL);
      cpu->PC += instruction.bytes;
      break;
    case 0xd9:
      /*
       * CMP - Compare (absolute, Y)
       *
       * Compares the contents of the accumulator with another value from
       * memory and sets the zero and carry flags as appropriate. The negative
       * flag is set in case bit 7 of the result of (A - value) is set.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        const uint8_t value = cpu_read_8b(cpu, address + cpu->Y);
        BIT_SET_IF(cpu->A >= value, cpu->P, FLAGS_BIT_CARRY);
        BIT_SET_IF(cpu->A == value, cpu->P, FLAGS_BIT_ZERO);
        BIT_SET_IF((cpu->A - value) & 0x80, cpu->P, FLAGS_BIT_NEGATIVE);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xe0:
      /*
       * CPX - Compare X Register (immediate)
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
    case 0xe1:
      /*
       * SBC - Subtract With Carry (indirect, X)
       *
       * Subtracts the contents of a memory location from the accumulator
       * together with the not of the carry bit. If overflow occurs, the carry
       * bit is clear.
       *
       * Note that SBC is simply ADC, with the value at the memory location
       * changed to one's complement. To see why this is, SBC is defined as:
       *
       *   SBC = A - v - B
       *
       * where `v` is some value in memory, and `B` is the borrow bit. B is
       * defined as the inverse of the carry flag: (1 - C). Thus:
       *
       *   SBC = A - v - (1 - C)
       *   SBC = A - v - (1 - C) + 256
       *   SBC = A - v - C + 255
       *   SBC = A + (255 - v) + C
       *
       * here, 255 - v is simply the one's complement of v. Note that adding 256
       * to an 8bit value does not change the 8bit value.
       */
      cpu_addc(cpu, ~(cpu_read_indirect_x(cpu, cpu->ram[cpu->PC + 1])));
      cpu->PC += instruction.bytes;
      break;
    case 0xe4:
      /*
       * CPX - Compare X Register (zero page)
       *
       * Compares the contents of the X register with another value from memory
       * and sets the zero and carry flags as appropriate. The negative flag is
       * set in case bit 7 of the result of (X - value) is set.
       */
      {
        const uint8_t value = cpu->ram[cpu->ram[cpu->PC + 1]];
        BIT_SET_IF(cpu->X >= value, cpu->P, FLAGS_BIT_CARRY);
        BIT_SET_IF(cpu->X == value, cpu->P, FLAGS_BIT_ZERO);
        BIT_SET_IF((cpu->X - value) & 0x80, cpu->P, FLAGS_BIT_NEGATIVE);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xe5:
      /*
       * SBC - Subtract With Carry (zero page)
       *
       * Subtracts the contents of a memory location from the accumulator
       * together with the not of the carry bit. If overflow occurs, the carry
       * bit is clear.
       *
       * Note that SBC is simply ADC, with the value at the memory location
       * changed to one's complement. To see why this is, SBC is defined as:
       *
       *   SBC = A - v - B
       *
       * where `v` is some value in memory, and `B` is the borrow bit. B is
       * defined as the inverse of the carry flag: (1 - C). Thus:
       *
       *   SBC = A - v - (1 - C)
       *   SBC = A - v - (1 - C) + 256
       *   SBC = A - v - C + 255
       *   SBC = A + (255 - v) + C
       *
       * here, 255 - v is simply the one's complement of v. Note that adding 256
       * to an 8bit value does not change the 8bit value.
       */
      cpu_addc(cpu, ~(cpu->ram[cpu->ram[cpu->PC + 1]]));
      cpu->PC += instruction.bytes;
      break;
    case 0xe6:
      /*
       * INC - Increment memory (zero page)
       *
       * Adds one to the value held at a specified memory location setting the
       * zero and negative flags as appropriate.
       */
      {
        uint8_t *value = cpu->ram + cpu->ram[cpu->PC + 1];
        (*value)++;
        cpu_set_zero_negative_flags(cpu, *value);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xe8:
      /*
       * INX - Increment X Register
       *
       * Increments the value in the X register, and sets the zero and negative
       * flags appropriately.
       */
      cpu->X++;
      cpu_set_zero_negative_flags(cpu, cpu->X);
      cpu->PC += instruction.bytes;
      break;
    case 0xe9:
      /*
       * SBC - Subtract With Carry (immediate)
       *
       * Subtracts the contents of a memory location from the accumulator
       * together with the not of the carry bit. If overflow occurs, the carry
       * bit is clear.
       *
       * Note that SBC is simply ADC, with the value at the memory location
       * changed to one's complement. To see why this is, SBC is defined as:
       *
       *   SBC = A - v - B
       *
       * where `v` is some value in memory, and `B` is the borrow bit. B is
       * defined as the inverse of the carry flag: (1 - C). Thus:
       *
       *   SBC = A - v - (1 - C)
       *   SBC = A - v - (1 - C) + 256
       *   SBC = A - v - C + 255
       *   SBC = A + (255 - v) + C
       *
       * here, 255 - v is simply the one's complement of v. Note that adding 256
       * to an 8bit value does not change the 8bit value.
       */
      cpu_addc(cpu, ~(cpu->ram[cpu->PC + 1]));
      cpu->PC += instruction.bytes;
      break;
    case 0xea:
      /*
       * NOP - No Operation
       *
       * The NOP instruction causes no changes to the processor other than the
       * normal incrementing of the program counter to the next instruction.
       */
      cpu->PC += instruction.bytes;
      break;
    case 0xec:
      /*
       * CPX - Compare X Register (absolute)
       *
       * Compares the contents of the X register with another value from memory
       * and sets the zero and carry flags as appropriate. The negative flag is
       * set in case bit 7 of the result of (X - value) is set.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        const uint8_t value = cpu->ram[address];
        BIT_SET_IF(cpu->X >= value, cpu->P, FLAGS_BIT_CARRY);
        BIT_SET_IF(cpu->X == value, cpu->P, FLAGS_BIT_ZERO);
        BIT_SET_IF((cpu->X - value) & 0x80, cpu->P, FLAGS_BIT_NEGATIVE);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xed:
      /*
       * SBC - Subtract With Carry (absolute)
       *
       * Subtracts the contents of a memory location from the accumulator
       * together with the not of the carry bit. If overflow occurs, the carry
       * bit is clear.
       *
       * Note that SBC is simply ADC, with the value at the memory location
       * changed to one's complement. To see why this is, SBC is defined as:
       *
       *   SBC = A - v - B
       *
       * where `v` is some value in memory, and `B` is the borrow bit. B is
       * defined as the inverse of the carry flag: (1 - C). Thus:
       *
       *   SBC = A - v - (1 - C)
       *   SBC = A - v - (1 - C) + 256
       *   SBC = A - v - C + 255
       *   SBC = A + (255 - v) + C
       *
       * here, 255 - v is simply the one's complement of v. Note that adding 256
       * to an 8bit value does not change the 8bit value.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        cpu_addc(cpu, ~cpu->ram[address]);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xee:
      /*
       * INC - Increment memory (absolute)
       *
       * Adds one to the value held at a specified memory location setting the
       * zero and negative flags as appropriate.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        uint8_t *value = cpu->ram + address;
        (*value)++;
        cpu_set_zero_negative_flags(cpu, *value);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xf0:
      /*
       * BEQ - Branch if Equal
       *
       * If the zero flag is set then add the relative displacement to the
       * program counter to cause a branch to a new location.
       */
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
    case 0xf1:
      /*
       * SBC - Subtract With Carry (indirect), Y
       *
       * Subtracts the contents of a memory location from the accumulator
       * together with the not of the carry bit. If overflow occurs, the carry
       * bit is clear.
       *
       * Note that SBC is simply ADC, with the value at the memory location
       * changed to one's complement. To see why this is, SBC is defined as:
       *
       *   SBC = A - v - B
       *
       * where `v` is some value in memory, and `B` is the borrow bit. B is
       * defined as the inverse of the carry flag: (1 - C). Thus:
       *
       *   SBC = A - v - (1 - C)
       *   SBC = A - v - (1 - C) + 256
       *   SBC = A - v - C + 255
       *   SBC = A + (255 - v) + C
       *
       * here, 255 - v is simply the one's complement of v. Note that adding 256
       * to an 8bit value does not change the 8bit value.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        cpu_addc(cpu, ~(cpu_read_indirect_y(cpu, operand)));
        cpu->cycle += cpu_page_cross(cpu_read_16b(cpu, operand), cpu->Y);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xf5:
      /*
       * SBC - Subtract With Carry (zero page, X)
       *
       * Subtracts the contents of a memory location from the accumulator
       * together with the not of the carry bit. If overflow occurs, the carry
       * bit is clear.
       *
       * Note that SBC is simply ADC, with the value at the memory location
       * changed to one's complement. To see why this is, SBC is defined as:
       *
       *   SBC = A - v - B
       *
       * where `v` is some value in memory, and `B` is the borrow bit. B is
       * defined as the inverse of the carry flag: (1 - C). Thus:
       *
       *   SBC = A - v - (1 - C)
       *   SBC = A - v - (1 - C) + 256
       *   SBC = A - v - C + 255
       *   SBC = A + (255 - v) + C
       *
       * here, 255 - v is simply the one's complement of v. Note that adding 256
       * to an 8bit value does not change the 8bit value.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        cpu_addc(cpu, ~(cpu_read_zero_page_x(cpu, operand)));
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xf6:
      /*
       * INC - Increment memory (zero page, X)
       *
       * Adds one to the value held at a specified memory location setting the
       * zero and negative flags as appropriate.
       */
      {
        const uint8_t operand = cpu_read_8b(cpu, cpu->PC + 1);
        uint8_t *value = cpu->ram + cpu_make_zero_page_x_offset(cpu, operand);
        (*value)++;
        cpu_set_zero_negative_flags(cpu, *value);
        cpu->PC += instruction.bytes;
      }
      break;
    case 0xf8:
      /*
       * SED - Set Decimal Flag
       *
       * Set the decimal flag to one.
       */
      BIT_SET(cpu->P, FLAGS_BIT_DECIMAL);
      cpu->PC += instruction.bytes;
      break;
    case 0xf9:
      /*
       * SBC - Subtract With Carry (absolute, Y)
       *
       * Subtracts the contents of a memory location from the accumulator
       * together with the not of the carry bit. If overflow occurs, the carry
       * bit is clear.
       *
       * Note that SBC is simply ADC, with the value at the memory location
       * changed to one's complement. To see why this is, SBC is defined as:
       *
       *   SBC = A - v - B
       *
       * where `v` is some value in memory, and `B` is the borrow bit. B is
       * defined as the inverse of the carry flag: (1 - C). Thus:
       *
       *   SBC = A - v - (1 - C)
       *   SBC = A - v - (1 - C) + 256
       *   SBC = A - v - C + 255
       *   SBC = A + (255 - v) + C
       *
       * here, 255 - v is simply the one's complement of v. Note that adding 256
       * to an 8bit value does not change the 8bit value.
       */
      {
        const Address address = cpu_read_16b(cpu, cpu->PC + 1);
        cpu_addc(cpu, ~cpu_read_8b(cpu, address + cpu->Y));
        cpu->PC += instruction.bytes;
      }
      break;
    default:
      nn_quit("Unknown opcode: %x", instruction.opcode);
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
