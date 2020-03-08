#ifndef NEPNES_CPU_H
#define NEPNES_CPU_H

#include <stdint.h>

/*
 * The NES CPU core is based on the 6502 processor and runs at approximately
 * 1.79 MHz (1.66 MHz in a PAL NES). It is made by Ricoh and lacks the MOS6502's
 * decimal mode. In the NTSC NES, the RP2A03 chip contains the CPU and APU; in
 * the PAL NES, the CPU and APU are contained within the RP2A07 chip.
 */

/*
 * Note, documentation on addressing modes is taken from the NES Hacker wiki,
 * at:
 * http://www.thealmightyguru.com/Games/Hacking/Wiki/index.php?title=Main_Page
 */

enum AddressingMode
{
  /*
   * Immediate Addressing is used when the operand's value is given in the
   * instruction itself. In 6502 assembly, this is indicated by an pound sign,
   * "#", before the operand.
   */
  AM_IMMEDIATE,
  /*
   * In Zero-Page Addressing, the operand is a memory address rather than a
   * value. As the name suggests, the memory must be on the zero-page of memory
   * (addresses $0000-$00FF). You only need to supply the low byte of the memory
   * address, the $00 high byte is automatically added by the processor.
   */
  AM_ZERO_PAGE,
  /*
   * Absolute addressing is very similar to zero-page addressing, except that
   * absolute addressing requires a full 2-byte address, and can access the full
   * range of the processor's memory ($0000-$FFFF).
   */
  AM_ABSOLUTE,
  /*
   * Implied Addressing occurs when there is no operand. The addressing mode is
   * implied by the instruction. Note that Accumulator Addressing is a special
   * type of Implied Addressing that only addresses the accumulator.
   */
  AM_IMPLIED,
  /*
   * Accumulator Addressing is a special type of Implied Addressing that only
   * addresses the accumulator.
   */
  AM_ACCUMULATOR,
  /*
   * In Indexed Addressing is similar to zero-page indexed addressing, except
   * that in Indexed Addressing you have to include full 2-byte address. In this
   * mode, the address is added to the value either by the X or Y index
   * register. Most opcodes support the X index register for the offset, but an
   * additional handful also support the Y index register.
   *
   * The benefit of Indexed addressing is that you can quickly loop through
   * memory by simply increasing or decreasing the offset.
   */
  AM_INDEXED,
  /*
   * Zero-page indexed addressing is similar to indexed addressing, except that
   * since it applies only to the zero-page, you only need to include 1-byte for
   * the address. In this mode, the address is added to the value either by the
   * X or Y index register. Most opcodes support the X index register for the
   * offset, but an additional handful also support the Y index register.
   *
   * The benefit of zero-page indexed addressing is that you can quickly loop
   * through memory by simply increasing or decreasing the offset.
   */
  AM_INDEXED_ZERO_PAGE,
  /*
   * Indirect Addressing reads a memory location from a two-byte pointer. This
   * particular addressing mode is used only by a special type of JMP. A pointer
   * is first stored in memory, and read for the address rather than using an
   * absolute position. There are two other indirect addressing modes,
   * pre-indexed indirect addressing and post-indexed indirect addressing.
   * Parenthesis are used to signify that the opcode is reading a pointer rather
   * than an absolute value.
   */
  AM_INDIRECT,
  /*
   * Pre-indexed indirect addressing is one of the three indirect addressing
   * modes of the 6502 processor. This mode accepts a zero-page address and adds
   * the contents of the X-register to get an address. The address is expected
   * to contain a 2-byte pointer to a memory address (ordered in little-endian).
   * The indirection is indicated by parenthesis in assembly language.
   *
   * This instruction is a three step process.
   *
   * 1. Sum the operand and the X-register to get the address to read from.
   * 2. Read the 2-byte address.
   * 3. Return the value found in the address.
   *
   * Keep in mind that this only operates on the zero page. The processor uses
   * wrap-around addition to sum up the operand and the X-register. Thus, if you
   * use an operand of $FF and an X-register of $10, the result is $0F, not $110
   * which you might expect.
   */
  AM_PRE_INDEXED_INDIRECT,
  /*
   * Post-Indexed Indirect Addressing is one of the three Indirect addressing
   * modes of the 6502 processor. This mode is similar to Pre-Indexed Indirect
   * Addressing, however, unlike the pre-indexed mode, where the X-register is
   * added to the operand prior to reading from memory, post-indexed mode adds
   * the Y-register after reading from memory. The address is expected to
   * contain a 2-byte pointer to a memory address (in little-endian, of course).
   * The indirection is indicated by parenthesis in assembly language, notice
   * that, unlike pre-indexed mode, they don't encompass the Y, signifying that
   * the addition occurs after the read.
   *
   * This instruction is a three step process.
   *
   * 1. Read the 2-byte address based on the operand.
   * 2. Sum the address and the Y-register to get the offset address.
   * 3. Return the value found in the offset address.
   */
  AM_POST_INDEXED_INDIRECT,
  /*
   * Relative addressing is used on the various Branch-On-Condition
   * instructions. A 1 byte signed operand is added to the program counter, and
   * the program continues execution from the new address. Because this value is
   * signed, values #00-#7F are positive, and values #FF-#80 are negative.
   *
   * Keep in mind that the program counter will be set to the address after the
   * branch instruction, so take this into account when calculating your new
   * position.
   *
   * Since branching works by checking a relevant status bit, make sure it is
   * set to the proper value prior to calling the branch instruction. This is
   * usually done with a CMP instruction.
   *
   * If you need to move the program counter to a location greater or less than
   * 127 bytes away from the current location, make a nearby JMP instruction,
   * and move the program counter to the JMP line.
   */
  AM_RELATIVE,
};

/* Enumeration of all possible operations that can be performed by the NES. */
enum Operation
{
  OP_AND,  // Logical AND
  OP_ASL,  // Arithmetic Shift Left
  OP_CPX,  // Compare X Register
  OP_INC,  // Increment Memory
};

/* Composes all relevant properties of an instruction, which is made up of an
 * encoding of the particular operation (opcode), a particular addressing mode
 * that indicates how the operands should be interpreted.
 */
struct Instruction
{
  /* The encoding of an instruction as seen by the CPU. */
  uint8_t opcode;
  /* The operation performed by the instruction. */
  enum Operation op;
  /* The number of bytes that are required for representing this opcode. */
  int bytes;
  /*
   * Addressing mode that indicates how the operands for this instruction should
   * interpreted.
   */
  enum AddressingMode addressing;
  /*
   * The number of CPU cylces it takes to execute the instruction encoded by
   * this opcode.
   */
  int cycles;
};

/* Converts the encoding of an instruction to an opcode representation. */
struct Instruction make_instruction(uint8_t opcode);
const char* operation_name(enum Operation op);

#endif
