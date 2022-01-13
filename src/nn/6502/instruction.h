#ifndef NEPNES_INSTRUCTION_H
#define NEPNES_INSTRUCTION_H

#include <stddef.h>
#include <stdint.h>

/* Enumeration of all possible operations that can be performed by the NES. */
enum Operation
{
  OP_ADC,  // Add With Carry
  OP_AND,  // Logical AND
  OP_ASL,  // Arithmetic Shift Left
  OP_BCC,  // Branch if Carry Clear
  OP_BCS,  // Branch if Carry Set
  OP_BEQ,  // Branch if Equal
  OP_BIT,  // Bit Test
  OP_BMI,  // Branch if Minus
  OP_BNE,  // Branch if Not Equal
  OP_BPL,  // Branch if Positive
  OP_BRK,  // Force Interrupt
  OP_BVC,  // Branch If Overflow Clear
  OP_BVS,  // Branch If Overflow Set
  OP_CLC,  // Clear Carry Flag
  OP_CLD,  // Clear Decimal Mode
  OP_CLI,  // Clear Interrupt Disable
  OP_CLV,  // Clear Overflow Flag
  OP_CMP,  // Compare
  OP_CPX,  // Compare X Register
  OP_CPY,  // Compare Y Register
  OP_DEC,  // Decrement Memory
  OP_DEX,  // Decrement X Register
  OP_DEY,  // Decrement Y Register
  OP_EOR,  // Exclusive OR
  OP_INC,  // Increment Memory
  OP_INX,  // Increment X Register
  OP_INY,  // Increment Y Register
  OP_JMP,  // Jump
  OP_JSR,  // Jump to Subroutine
  OP_LDA,  // Load Accumulator
  OP_LDX,  // Load X Register
  OP_LDY,  // Load Y Register
  OP_LSR,  // Logical Shift Right
  OP_NOP,  // No Operation
  OP_ORA,  // Logical Accumulator OR
  OP_PHA,  // Push Accumulator
  OP_PHP,  // Push Processor Status
  OP_PLA,  // Pull Accumulator
  OP_PLP,  // Pull Processor Status
  OP_ROL,  // Rotate Left
  OP_ROR,  // Rotate Right
  OP_RTI,  // Return from Interrupt
  OP_RTS,  // Return from Subroutine
  OP_SBC,  // Subtract With Carry
  OP_SEC,  // Set Carry Flag
  OP_SED,  // Set Decimal Flag
  OP_SEI,  // Set Interrupt Disable
  OP_STA,  // Store Accumulator
  OP_STX,  // Store X Register
  OP_STY,  // Store Y Register
  OP_TAX,  // Transfer Accumulator to X
  OP_TAY,  // Transfer Accumulator to Y
  OP_TSX,  // Transfer Stack Pointer to X
  OP_TXA,  // Transfer X to Accumulator
  OP_TXS,  // Transfer X to Stack Pointer
  OP_TYA,  // Transfer Y to Accumulator
};

/*
 * Note, documentation on addressing modes is taken from the NES Hacker wiki,
 * at:
 * http://www.thealmightyguru.com/Games/Hacking/Wiki/index.php?title=Main_Page
 */
enum AddressingMode
{
  /*
   * Absolute addressing is very similar to zero-page addressing, except that
   * absolute addressing requires a full 2-byte address, and can access the full
   * range of the processor's memory ($0000-$FFFF).
   */
  AM_ABSOLUTE,
  /*
   * Similar to absolute addressing, except that the value in the X register is
   * added to the provided 16-bit address to compute the final address.
   */
  AM_ABSOLUTE_X,
  /*
   * Similar to absolute addressing, except that the value in the Y register is
   * added to the provided 16-bit address to compute the final address.
   */
  AM_ABSOLUTE_Y,
  /*
   * Accumulator Addressing is a special type of Implied Addressing that only
   * addresses the accumulator.
   */
  AM_ACCUMULATOR,
  /*
   * Immediate Addressing is used when the operand's value is given in the
   * instruction itself. In 6502 assembly, this is indicated by an pound sign,
   * "#", before the operand.
   */
  AM_IMMEDIATE,
  /*
   * Implied Addressing occurs when there is no operand. The addressing mode is
   * implied by the instruction. Note that Accumulator Addressing is a special
   * type of Implied Addressing that only addresses the accumulator.
   */
  AM_IMPLIED,
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
  AM_INDIRECT_X,
  /*
   * Post-indexed indirect addressing is one of the three indirect addressing
   * modes of the 6502 processor. This mode is similar to pre-indexed indirect
   * addressing mode (AM_INDIRECT_X), however, unlike the pre-indexed mode,
   * where the X-register is added to the operand prior to reading from memory,
   * post-indexed mode adds the Y-register after reading from memory. The
   * address is expected to contain a 2-byte pointer to a memory address (in
   * little-endian, of course). The indirection is indicated by parenthesis in
   * assembly language, notice that, unlike pre-indexed mode, they don't
   * encompass the Y, signifying that the addition occurs after the read.
   *
   * This instruction is a three step process.
   *
   * 1. Read the 2-byte address based on the operand.
   * 2. Sum the address and the Y-register to get the offset address.
   * 3. Return the value found in the offset address.
   */
  AM_INDIRECT_Y,
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
  /*
   * In Zero-Page Addressing, the operand is a memory address rather than a
   * value. As the name suggests, the memory must be on the zero-page of memory
   * (addresses $0000-$00FF). You only need to supply the low byte of the memory
   * address, the $00 high byte is automatically added by the processor.
   */
  AM_ZERO_PAGE,
  /*
   * Zero-page indexed addressing is similar to indexed addressing, except that
   * since it applies only to the zero-page, you only need to include 1-byte for
   * the address. In this mode, the address is added to the value in the X index
   * register.
   *
   * The benefit of zero-page indexed addressing is that you can quickly loop
   * through memory by simply increasing or decreasing the offset.
   */
  AM_ZERO_PAGE_X,
  /*
   * Equivalent to the `AM_ZERO_PAGE_X` addressing mode, except that the value
   * in the Y index register is added to the provided 1-byte address.
   */
  AM_ZERO_PAGE_Y,
};

/*
 * Composes all relevant properties of an instruction, which is made up of an
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
  /* Indicates how the operands for this instruction should interpreted. */
  enum AddressingMode addressing_mode;
  /* The number of CPU cycles it takes to execute this instruction. */
  int cycles;
};

struct Instruction make_instruction(uint8_t opcode);
size_t instruction_size(uint8_t opcode);
uint8_t *advance_instruction(uint8_t *pc, int n);
uint8_t *next_instruction(uint8_t *pc);

/*
 * Size of the buffer that holds the textual representation of an instruction.
 */
enum
{
  INSTRUCTION_BUFSIZE = 14
};

enum InstructionLayout
{
  IL_NES_DISASM,
  IL_NINTENDULATOR,
};

struct Cpu;

const char *instruction_print(struct Instruction *ins, uint32_t encoding);
const char *instruction_print_layout(struct Instruction *ins, uint32_t encoding,
                                     enum InstructionLayout layout, struct Cpu *cpu);

#endif
