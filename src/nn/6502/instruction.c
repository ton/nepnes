#include "instruction.h"

#include <nn/6502/cpu.h>
#include <nn/std/util.h>

#include <stdio.h>

static const char *operation_mnemonic[] = {
    [OP_ADC] = "ADC", [OP_AND] = "AND", [OP_ASL] = "ASL", [OP_BCC] = "BCC", [OP_BCS] = "BCS",
    [OP_BEQ] = "BEQ", [OP_BIT] = "BIT", [OP_BMI] = "BMI", [OP_BNE] = "BNE", [OP_BPL] = "BPL",
    [OP_BRK] = "BRK", [OP_BVC] = "BVC", [OP_BVS] = "BVS", [OP_CLC] = "CLC", [OP_CLD] = "CLD",
    [OP_CLI] = "CLI", [OP_CLV] = "CLV", [OP_CMP] = "CMP", [OP_CPX] = "CPX", [OP_CPY] = "CPY",
    [OP_DEC] = "DEC", [OP_DEX] = "DEX", [OP_DEY] = "DEY", [OP_EOR] = "EOR", [OP_INC] = "INC",
    [OP_INX] = "INX", [OP_INY] = "INY", [OP_JMP] = "JMP", [OP_JSR] = "JSR", [OP_LDA] = "LDA",
    [OP_LDX] = "LDX", [OP_LDY] = "LDY", [OP_LSR] = "LSR", [OP_NOP] = "NOP", [OP_ORA] = "ORA",
    [OP_PHA] = "PHA", [OP_PHP] = "PHP", [OP_PLA] = "PLA", [OP_PLP] = "PLP", [OP_ROL] = "ROL",
    [OP_ROR] = "ROR", [OP_RTI] = "RTI", [OP_RTS] = "RTS", [OP_SBC] = "SBC", [OP_SEC] = "SEC",
    [OP_SED] = "SED", [OP_SEI] = "SEI", [OP_STA] = "STA", [OP_STX] = "STX", [OP_STY] = "STY",
    [OP_TAX] = "TAX", [OP_TAY] = "TAY", [OP_TSX] = "TSX", [OP_TXA] = "TXA", [OP_TXS] = "TXS",
    [OP_TYA] = "TYA", [OP_IGN] = "IGN", [OP_SKB] = "SKB",
};

static const struct Instruction instructions[256] = {
    {0x00, OP_BRK, 1, AM_IMPLIED, 7, true},
    {0x01, OP_ORA, 2, AM_INDIRECT_X, 6, true},
    {0},
    {0},
    {0x04, OP_IGN, 2, AM_ZERO_PAGE, 3, false},
    {0x05, OP_ORA, 2, AM_ZERO_PAGE, 3, true},
    {0x06, OP_ASL, 2, AM_ZERO_PAGE, 5, true},
    {0},
    {0x08, OP_PHP, 1, AM_IMPLIED, 3, true},
    {0x09, OP_ORA, 2, AM_IMMEDIATE, 2, true},
    {0x0a, OP_ASL, 1, AM_ACCUMULATOR, 2, true},
    {0},
    {0x0c, OP_IGN, 3, AM_ABSOLUTE, 4, false},
    {0x0d, OP_ORA, 3, AM_ABSOLUTE, 4, true},
    {0x0e, OP_ASL, 3, AM_ABSOLUTE, 6, true},
    {0},
    {0x10, OP_BPL, 2, AM_RELATIVE, 2 /* +1 on branch, +2 on page cross */, true},
    {0x11, OP_ORA, 2, AM_INDIRECT_Y, 5 /* +1 on page cross */, true},
    {0},
    {0},
    {0x14, OP_IGN, 2, AM_ZERO_PAGE_X, 4, false},
    {0x15, OP_ORA, 2, AM_ZERO_PAGE_X, 4, true},
    {0x16, OP_ASL, 2, AM_ZERO_PAGE_X, 6, true},
    {0},
    {0x18, OP_CLC, 1, AM_IMPLIED, 2, true},
    {0x19, OP_ORA, 3, AM_ABSOLUTE_Y, 4 /* +1 on page cross */, true},
    {0x1a, OP_NOP, 1, AM_IMPLIED, 2, false},
    {0},
    {0x1c, OP_IGN, 3, AM_ABSOLUTE_X, 4 /* +1 on page cross */, false},
    {0x1d, OP_ORA, 3, AM_ABSOLUTE_X, 4 /* +1 on page cross */, true},
    {0x1e, OP_ASL, 3, AM_ABSOLUTE_X, 7, true},
    {0},
    {0x20, OP_JSR, 3, AM_ABSOLUTE, 6, true},
    {0x21, OP_AND, 2, AM_INDIRECT_X, 6, true},
    {0},
    {0},
    {0x24, OP_BIT, 2, AM_ZERO_PAGE, 3, true},
    {0x25, OP_AND, 2, AM_ZERO_PAGE, 3, true},
    {0x26, OP_ROL, 2, AM_ZERO_PAGE, 5, true},
    {0},
    {0x28, OP_PLP, 1, AM_IMPLIED, 4, true},
    {0x29, OP_AND, 2, AM_IMMEDIATE, 2, true},
    {0x2a, OP_ROL, 1, AM_ACCUMULATOR, 2, true},
    {0},
    {0x2c, OP_BIT, 3, AM_ABSOLUTE, 4, true},
    {0x2d, OP_AND, 3, AM_ABSOLUTE, 4, true},
    {0x2e, OP_ROL, 3, AM_ABSOLUTE, 6, true},
    {0},
    {0x30, OP_BMI, 2, AM_RELATIVE, 2 /* +1 on branch, +2 on page cross */, true},
    {0x31, OP_AND, 2, AM_INDIRECT_Y, 5 /* +1 on page cross */, true},
    {0},
    {0},
    {0x34, OP_IGN, 2, AM_ZERO_PAGE_X, 4, false},
    {0x35, OP_AND, 2, AM_ZERO_PAGE_X, 4, true},
    {0x36, OP_ROL, 2, AM_ZERO_PAGE_X, 6, true},
    {0},
    {0x38, OP_SEC, 1, AM_IMPLIED, 2, true},
    {0x39, OP_AND, 3, AM_ABSOLUTE_Y, 4 /* +1 on page cross */, true},
    {0x3a, OP_NOP, 1, AM_IMPLIED, 2, false},
    {0},
    {0x3c, OP_IGN, 3, AM_ABSOLUTE_X, 4 /* +1 on page cross */, false},
    {0x3d, OP_AND, 3, AM_ABSOLUTE_X, 4 /* +1 on page cross */, true},
    {0x3e, OP_ROL, 3, AM_ABSOLUTE_X, 7, true},
    {0},
    {0x40, OP_RTI, 1, AM_IMPLIED, 6, true},
    {0x41, OP_EOR, 2, AM_INDIRECT_X, 6, true},
    {0},
    {0},
    {0x44, OP_IGN, 2, AM_ZERO_PAGE, 3, false},
    {0x45, OP_EOR, 2, AM_ZERO_PAGE, 3, true},
    {0x46, OP_LSR, 2, AM_ZERO_PAGE, 5, true},
    {0},
    {0x48, OP_PHA, 1, AM_IMPLIED, 3, true},
    {0x49, OP_EOR, 2, AM_IMMEDIATE, 2, true},
    {0x4a, OP_LSR, 1, AM_ACCUMULATOR, 2, true},
    {0},
    {0x4c, OP_JMP, 3, AM_ABSOLUTE, 3, true},
    {0x4d, OP_EOR, 3, AM_ABSOLUTE, 4, true},
    {0x4e, OP_LSR, 3, AM_ABSOLUTE, 6, true},
    {0},
    {0x50, OP_BVC, 2, AM_RELATIVE, 2 /* +1 on branch, +2 on page cross */, true},
    {0x51, OP_EOR, 2, AM_INDIRECT_Y, 5 /* +1 on page cross */, true},
    {0},
    {0},
    {0x54, OP_IGN, 2, AM_ZERO_PAGE_X, 4, false},
    {0x55, OP_EOR, 2, AM_ZERO_PAGE_X, 4, true},
    {0x56, OP_LSR, 2, AM_ZERO_PAGE_X, 6, true},
    {0},
    {0x58, OP_CLI, 1, AM_IMPLIED, 2, true},
    {0x59, OP_EOR, 3, AM_ABSOLUTE_Y, 4 /* +1 on page cross */, true},
    {0x5a, OP_NOP, 1, AM_IMPLIED, 2, false},
    {0},
    {0x5c, OP_IGN, 3, AM_ABSOLUTE_X, 4 /* +1 on page cross */, false},
    {0x5d, OP_EOR, 3, AM_ABSOLUTE_X, 4 /* +1 on page cross */, true},
    {0x5e, OP_LSR, 3, AM_ABSOLUTE_X, 7, true},
    {0},
    {0x60, OP_RTS, 1, AM_IMPLIED, 6, true},
    {0x61, OP_ADC, 2, AM_INDIRECT_X, 6, true},
    {0},
    {0},
    {0x64, OP_IGN, 2, AM_ZERO_PAGE, 3, false},
    {0x65, OP_ADC, 2, AM_ZERO_PAGE, 3, true},
    {0x66, OP_ROR, 2, AM_ZERO_PAGE, 5, true},
    {0},
    {0x68, OP_PLA, 1, AM_IMPLIED, 4, true},
    {0x69, OP_ADC, 2, AM_IMMEDIATE, 2, true},
    {0x6a, OP_ROR, 1, AM_ACCUMULATOR, 2, true},
    {0},
    {0x6c, OP_JMP, 3, AM_INDIRECT, 5, true},
    {0x6d, OP_ADC, 3, AM_ABSOLUTE, 4, true},
    {0x6e, OP_ROR, 3, AM_ABSOLUTE, 6, true},
    {0},
    {0x70, OP_BVS, 2, AM_RELATIVE, 2 /* +1 on branch, +2 on page cross */, true},
    {0x71, OP_ADC, 2, AM_INDIRECT_Y, 5 /* +1 on page cross */, true},
    {0},
    {0},
    {0x54, OP_IGN, 2, AM_ZERO_PAGE_X, 4, false},
    {0x75, OP_ADC, 2, AM_ZERO_PAGE_X, 4, true},
    {0x76, OP_ROR, 2, AM_ZERO_PAGE_X, 6, true},
    {0},
    {0x78, OP_SEI, 1, AM_IMPLIED, 2, true},
    {0x79, OP_ADC, 3, AM_ABSOLUTE_Y, 4 /* +1 on page cross */, true},
    {0x7a, OP_NOP, 1, AM_IMPLIED, 2, false},
    {0},
    {0x7c, OP_IGN, 3, AM_ABSOLUTE_X, 4 /* +1 on page cross */, false},
    {0x7d, OP_ADC, 3, AM_ABSOLUTE_X, 4 /* +1 on page cross */, true},
    {0x7e, OP_ROR, 3, AM_ABSOLUTE_X, 7, true},
    {0},
    {0x80, OP_SKB, 2, AM_IMMEDIATE, 2, false},
    {0x81, OP_STA, 2, AM_INDIRECT_X, 6, true},
    {0},
    {0},
    {0x84, OP_STY, 2, AM_ZERO_PAGE, 3, true},
    {0x85, OP_STA, 2, AM_ZERO_PAGE, 3, true},
    {0x86, OP_STX, 2, AM_ZERO_PAGE, 3, true},
    {0},
    {0x88, OP_DEY, 1, AM_IMPLIED, 2, true},
    {0},
    {0x8a, OP_TXA, 1, AM_IMPLIED, 2, true},
    {0},
    {0x8c, OP_STY, 3, AM_ABSOLUTE, 4, true},
    {0x8d, OP_STA, 3, AM_ABSOLUTE, 4, true},
    {0x8e, OP_STX, 3, AM_ABSOLUTE, 4, true},
    {0},
    {0x90, OP_BCC, 2, AM_RELATIVE, 2 /* +1 on branch, +2 on page cross */, true},
    {0x91, OP_STA, 2, AM_INDIRECT_Y, 6, true},
    {0},
    {0},
    {0x94, OP_STY, 2, AM_ZERO_PAGE_X, 4, true},
    {0x95, OP_STA, 2, AM_ZERO_PAGE_X, 4, true},
    {0x96, OP_STX, 2, AM_ZERO_PAGE_Y, 4, true},
    {0},
    {0x98, OP_TYA, 1, AM_IMPLIED, 2, true},
    {0x99, OP_STA, 3, AM_ABSOLUTE_Y, 5, true},
    {0x9a, OP_TXS, 1, AM_IMPLIED, 2, true},
    {0},
    {0},
    {0x9d, OP_STA, 3, AM_ABSOLUTE_X, 5, true},
    {0},
    {0},
    {0xa0, OP_LDY, 2, AM_IMMEDIATE, 2, true},
    {0xa1, OP_LDA, 2, AM_INDIRECT_X, 6, true},
    {0xa2, OP_LDX, 2, AM_IMMEDIATE, 2, true},
    {0},
    {0xa4, OP_LDY, 2, AM_ZERO_PAGE, 3, true},
    {0xa5, OP_LDA, 2, AM_ZERO_PAGE, 3, true},
    {0xa6, OP_LDX, 2, AM_ZERO_PAGE, 3, true},
    {0},
    {0xa8, OP_TAY, 1, AM_IMPLIED, 2, true},
    {0xa9, OP_LDA, 2, AM_IMMEDIATE, 2, true},
    {0xaa, OP_TAX, 1, AM_IMPLIED, 2, true},
    {0},
    {0xac, OP_LDY, 3, AM_ABSOLUTE, 4, true},
    {0xad, OP_LDA, 3, AM_ABSOLUTE, 4, true},
    {0xae, OP_LDX, 3, AM_ABSOLUTE, 4, true},
    {0},
    {0xb0, OP_BCS, 2, AM_RELATIVE, 2 /* +1 on branch, +2 on page cross */, true},
    {0xb1, OP_LDA, 2, AM_INDIRECT_Y, 5 /* +1 on page cross */, true},
    {0},
    {0},
    {0xb4, OP_LDY, 2, AM_ZERO_PAGE_X, 4, true},
    {0xb5, OP_LDA, 2, AM_ZERO_PAGE_X, 4, true},
    {0xb6, OP_LDX, 2, AM_ZERO_PAGE_Y, 4, true},
    {0},
    {0xb8, OP_CLV, 1, AM_IMPLIED, 2, true},
    {0xb9, OP_LDA, 3, AM_ABSOLUTE_Y, 4 /* +1 on page cross */, true},
    {0xba, OP_TSX, 1, AM_IMPLIED, 2, true},
    {0},
    {0xbc, OP_LDY, 3, AM_ABSOLUTE_X, 4 /* +1 on page cross */, true},
    {0xbd, OP_LDA, 3, AM_ABSOLUTE_X, 4 /* +1 on page cross */, true},
    {0xbe, OP_LDX, 3, AM_ABSOLUTE_Y, 4 /* +1 on page cross */, true},
    {0},
    {0xc0, OP_CPY, 2, AM_IMMEDIATE, 2, true},
    {0xc1, OP_CMP, 2, AM_INDIRECT_X, 6, true},
    {0},
    {0},
    {0xc4, OP_CPY, 2, AM_ZERO_PAGE, 3, true},
    {0xc5, OP_CMP, 2, AM_ZERO_PAGE, 3, true},
    {0xc6, OP_DEC, 2, AM_ZERO_PAGE, 5, true},
    {0},
    {0xc8, OP_INY, 1, AM_IMPLIED, 2, true},
    {0xc9, OP_CMP, 2, AM_IMMEDIATE, 2, true},
    {0xca, OP_DEX, 1, AM_IMPLIED, 2, true},
    {0},
    {0xcc, OP_CPY, 3, AM_ABSOLUTE, 4, true},
    {0xcd, OP_CMP, 3, AM_ABSOLUTE, 4, true},
    {0xce, OP_DEC, 3, AM_ABSOLUTE, 6, true},
    {0},
    {0xd0, OP_BNE, 2, AM_RELATIVE, 2 /* +1 on branch, +2 on page cross */, true},
    {0xd1, OP_CMP, 2, AM_INDIRECT_Y, 5 /* +1 on page cross */, true},
    {0},
    {0},
    {0xd4, OP_IGN, 2, AM_ZERO_PAGE_X, 4, false},
    {0xd5, OP_CMP, 2, AM_ZERO_PAGE_X, 4, true},
    {0xd6, OP_DEC, 2, AM_ZERO_PAGE_X, 6, true},
    {0},
    {0xd8, OP_CLD, 1, AM_IMPLIED, 2, true},
    {0xd9, OP_CMP, 3, AM_ABSOLUTE_Y, 4 /* +1 on page cross */, true},
    {0xda, OP_NOP, 1, AM_IMPLIED, 2, false},
    {0},
    {0xdc, OP_IGN, 3, AM_ABSOLUTE_X, 4 /* +1 on page cross */, false},
    {0xdd, OP_CMP, 3, AM_ABSOLUTE_X, 4 /* +1 on page cross */, true},
    {0xde, OP_DEC, 3, AM_ABSOLUTE_X, 7, true},
    {0},
    {0xe0, OP_CPX, 2, AM_IMMEDIATE, 2, true},
    {0xe1, OP_SBC, 2, AM_INDIRECT_X, 6, true},
    {0},
    {0},
    {0xe4, OP_CPX, 2, AM_ZERO_PAGE, 3, true},
    {0xe5, OP_SBC, 2, AM_ZERO_PAGE, 3, true},
    {0xe6, OP_INC, 2, AM_ZERO_PAGE, 5, true},
    {0},
    {0xe8, OP_INX, 1, AM_IMPLIED, 2, true},
    {0xe9, OP_SBC, 2, AM_IMMEDIATE, 2, true},
    {0xea, OP_NOP, 1, AM_IMPLIED, 2, true},
    {0},
    {0xec, OP_CPX, 3, AM_ABSOLUTE, 4, true},
    {0xed, OP_SBC, 3, AM_ABSOLUTE, 4, true},
    {0xee, OP_INC, 3, AM_ABSOLUTE, 6, true},
    {0},
    {0xf0, OP_BEQ, 2, AM_RELATIVE, 2 /* +1 on branch, +2 on page cross */, true},
    {0xf1, OP_SBC, 2, AM_INDIRECT_Y, 5 /* +1 on page cross */, true},
    {0},
    {0},
    {0xf4, OP_IGN, 2, AM_ZERO_PAGE_X, 4, false},
    {0xf5, OP_SBC, 2, AM_ZERO_PAGE_X, 4, true},
    {0xf6, OP_INC, 2, AM_ZERO_PAGE_X, 6, true},
    {0},
    {0xf8, OP_SED, 1, AM_IMPLIED, 2, true},
    {0xf9, OP_SBC, 3, AM_ABSOLUTE_Y, 4 /* +1 on page cross */, true},
    {0xfa, OP_NOP, 1, AM_IMPLIED, 2, false},
    {0},
    {0xfc, OP_IGN, 3, AM_ABSOLUTE_X, 4 /* +1 on page cross */, false},
    {0xfd, OP_SBC, 3, AM_ABSOLUTE_X, 4 /* +1 on page cross */, true},
    {0xfe, OP_INC, 3, AM_ABSOLUTE_X, 7, true},
    {0}};

/*
 * Returns the assembly token for the given operation.
 */
static const char *operation_name(enum Operation op)
{
  return operation_mnemonic[op];
}

/*
 * Given an instruction encoding, reads its 8-bit operand.
 */
static uint8_t read_8b_op(Encoding encoding)
{
  return (encoding >> 8) & 0xff;
}

/*
 * Given an instruction encoding, reads its signed 8-bit operand.
 */
static int8_t read_signed_8b_op(Encoding encoding)
{
  return (encoding >> 8) & 0xff;
}

/*
 * Given an instruction encoding, reads its 16-bit operand.
 */
static uint16_t read_16b_op(Encoding encoding)
{
  return (encoding >> 8) & 0xffff;
}

/*
 * Constructs an opcode containing all information on the given opcode encoding,
 * like the number of bytes it occupies in memory, the number of cycles it takes
 * to execute, and the addressing mode it uses.
 */
struct Instruction make_instruction(uint8_t opcode)
{
  return instructions[opcode];
}

/*
 * Returns the number of bytes required for the encoding of the given opcode. In
 * case the given opcode is not a valid opcode, returns 1.
 */
size_t instruction_size(uint8_t opcode)
{
  const struct Instruction *ins = &instructions[opcode];
  return ins->opcode == opcode ? ins->bytes : 1;
}

/*
 * Returns the encoding of the given instruction, which is at most three bytes
 * long.
 */
Encoding instruction_read_encoding(const uint8_t *buf, int bytes)
{
  if (bytes == 1)
  {
    return *buf;
  }
  else if (bytes == 2)
  {
    return *buf + (*(buf + 1) << 8);
  }
  return *buf + (*(buf + 1) << 8) + (*(buf + 2) << 16);
}

/*
 * Advances the given program counter n instructions, and returns the new
 * program counter. n may be negative to rewind the program counter.
 */
uint8_t *advance_instruction(uint8_t *pc, int n)
{
  int step_sign = n < 0 ? -1 : 1;
  while (n != 0)
  {
    pc += step_sign * instruction_size(*pc);
    n -= step_sign;
  }
  return pc;
}

/*
 * Advances the given program counter n instructions, and returns the new
 * program counter. n may be negative to rewind the program counter.
 */
uint8_t *next_instruction(uint8_t *pc)
{
  const uint8_t opcode = *pc;
  const struct Instruction *ins = &instructions[opcode];
  return pc + (ins->opcode == opcode ? ins->bytes : 1);
}

/*
 * See `instruction_print_layout`. Prints an instruction to some statically
 * allocated buffer using `nes-disasm` instruction layout.
 */
const char *instruction_print(struct Instruction *ins, Encoding encoding)
{
  return instruction_print_layout(ins, encoding, IL_NES_DISASM, NULL);
}

/*
 * Writes the assembly representation for the given instruction to a statically
 * allocated buffer using the given instruction layout. Right now, two different
 * layout types are supported, the one from `nes-disasm` and from
 * 'Nintendulator'. The Nintendulator mode requires access to RAM, since it will
 * print out values that are stored in the zero page.
 */
const char *instruction_print_layout(struct Instruction *ins, Encoding encoding,
                                     enum InstructionLayout layout, struct Cpu *cpu)
{
  static char buffer[32];

  /* TODO(ton): Nintendulator prints extra information next to the assembly
   * depending on the addressing mode of the instruction. This can be
   * implemented in a more general way instead of the ad-hoc approach that is
   * taken below.
   */

  const char *op_name = layout == IL_NINTENDULATOR && (ins->op == OP_IGN || ins->op == OP_SKB)
                            ? "NOP"
                            : operation_name(ins->op);

  switch (ins->addressing_mode)
  {
    case AM_ABSOLUTE:
    {
      const bool print_address_value =
          layout == IL_NINTENDULATOR &&
          (ins->op == OP_STX || ins->op == OP_LDX || ins->op == OP_LDA || ins->op == OP_STA ||
           ins->op == OP_LDY || ins->op == OP_STY || ins->op == OP_BIT || ins->op == OP_ORA ||
           ins->op == OP_AND || ins->op == OP_EOR || ins->op == OP_ADC || ins->op == OP_CMP ||
           ins->op == OP_SBC || ins->op == OP_CPX || ins->op == OP_CPY || ins->op == OP_LSR ||
           ins->op == OP_ASL || ins->op == OP_ROR || ins->op == OP_ROL || ins->op == OP_INC ||
           ins->op == OP_DEC || ins->op == OP_IGN);
      if (print_address_value)
      {
        snprintf(buffer, sizeof buffer, "%s $%04X = %02X", op_name, read_16b_op(encoding),
                 cpu->ram[read_16b_op(encoding)]);
      }
      else
      {
        snprintf(buffer, sizeof buffer, "%s $%04X", op_name, read_16b_op(encoding));
      }
    }
    break;
    case AM_ABSOLUTE_X:
    {
      const bool print_address_value =
          layout == IL_NINTENDULATOR &&
          (ins->op == OP_LDY || ins->op == OP_ORA || ins->op == OP_AND || ins->op == OP_EOR ||
           ins->op == OP_ADC || ins->op == OP_CMP || ins->op == OP_SBC || ins->op == OP_LDA ||
           ins->op == OP_STA || ins->op == OP_LSR || ins->op == OP_ASL || ins->op == OP_ROR ||
           ins->op == OP_ROL || ins->op == OP_INC || ins->op == OP_DEC || ins->op == OP_IGN);
      if (print_address_value)
      {
        const Address address = read_16b_op(encoding) + cpu->X;
        snprintf(buffer, sizeof buffer, "%s $%04X,X @ %04X = %02X", op_name, read_16b_op(encoding),
                 address, cpu_read_8b(cpu, address));
      }
      else
      {
        snprintf(buffer, sizeof buffer, "%s $%04X,X", op_name, read_16b_op(encoding));
      }
    }
    break;
    case AM_ABSOLUTE_Y:
    {
      const bool print_address_value =
          layout == IL_NINTENDULATOR &&
          (ins->op == OP_LDA || ins->op == OP_ORA || ins->op == OP_AND || ins->op == OP_EOR ||
           ins->op == OP_ADC || ins->op == OP_CMP || ins->op == OP_SBC || ins->op == OP_STA ||
           ins->op == OP_LDY || ins->op == OP_LDX);
      if (print_address_value)
      {
        const Address address = read_16b_op(encoding) + cpu->Y;
        snprintf(buffer, sizeof buffer, "%s $%04X,Y @ %04X = %02X", op_name, read_16b_op(encoding),
                 address, cpu_read_8b(cpu, address));
      }
      else
      {
        snprintf(buffer, sizeof buffer, "%s $%04X,Y", op_name, read_16b_op(encoding));
      }
    }
    break;
    case AM_ACCUMULATOR:
      snprintf(buffer, sizeof buffer, "%s A", op_name);
      break;
    case AM_IMMEDIATE:
      snprintf(buffer, sizeof buffer, "%s #$%02X", op_name, read_8b_op(encoding));
      break;
    case AM_IMPLIED:
      snprintf(buffer, sizeof buffer, "%s", op_name);
      break;
    case AM_INDIRECT:
      if (layout == IL_NINTENDULATOR)
      {
        const Address address = read_16b_op(encoding);
        snprintf(buffer, sizeof buffer, "%s ($%04X) = %04X", op_name, address,
                 cpu_read_indirect_16b(cpu, address));
      }
      else
      {
        snprintf(buffer, sizeof buffer, "%s ($%04X)", op_name, read_16b_op(encoding));
      }
      break;
    case AM_INDIRECT_X:
    {
      const uint8_t operand = read_8b_op(encoding);

      const bool print_address_value =
          layout == IL_NINTENDULATOR &&
          (ins->op == OP_LDA || ins->op == OP_STA || ins->op == OP_ORA || ins->op == OP_AND ||
           ins->op == OP_EOR || ins->op == OP_ADC || ins->op == OP_CMP || ins->op == OP_SBC);
      if (print_address_value)
      {
        const Address ptr = cpu_read_indirect_x_address(cpu, operand);
        const uint8_t data = cpu_read_indirect_x(cpu, operand);

        snprintf(buffer, sizeof buffer, "%s ($%02X,X) @ %02X = %04X = %02X", op_name, operand,
                 (uint8_t)(operand + cpu->X), ptr, data);
      }
      else
      {
        snprintf(buffer, sizeof buffer, "%s ($%02X,X)", op_name, operand);
      }
    }
    break;
    case AM_INDIRECT_Y:
    {
      const uint8_t operand = read_8b_op(encoding);

      const bool print_address_value =
          layout == IL_NINTENDULATOR &&
          (ins->op == OP_LDA || ins->op == OP_ORA || ins->op == OP_AND || ins->op == OP_EOR ||
           ins->op == OP_ADC || ins->op == OP_CMP || ins->op == OP_SBC || ins->op == OP_STA);
      if (print_address_value)
      {
        const Address ptr = cpu_read_indirect_y_address(cpu, operand);
        const uint8_t data = cpu_read_indirect_y(cpu, operand);

        snprintf(buffer, sizeof buffer, "%s ($%02X),Y = %04X @ %04X = %02X", op_name, operand,
                 cpu_read_indirect_address(cpu, operand), ptr, data);
      }
      else
      {
        snprintf(buffer, sizeof buffer, "%s ($%02X),Y", op_name, operand);
      }
    }
    break;
    case AM_RELATIVE:
      switch (layout)
      {
        case IL_NES_DISASM:
          snprintf(buffer, sizeof buffer, "%s $%02X (%d)", op_name, read_8b_op(encoding),
                   read_signed_8b_op(encoding));
          break;
        case IL_NINTENDULATOR:
          snprintf(buffer, sizeof buffer, "%s $%02X", op_name,
                   cpu->PC + ins->bytes + (read_signed_8b_op(encoding)));
          break;
      }
      break;
    case AM_ZERO_PAGE:
      switch (layout)
      {
        case IL_NES_DISASM:
          snprintf(buffer, sizeof buffer, "%s $%02X", op_name, read_8b_op(encoding));
          break;
        case IL_NINTENDULATOR:
          snprintf(buffer, sizeof buffer, "%s $%02X = %02X", op_name, read_8b_op(encoding),
                   cpu->ram[read_8b_op(encoding)]);
          break;
      }
      break;
    case AM_ZERO_PAGE_X:
    {
      const bool print_address_value =
          layout == IL_NINTENDULATOR &&
          (ins->op == OP_LDY || ins->op == OP_STY || ins->op == OP_ORA || ins->op == OP_AND ||
           ins->op == OP_EOR || ins->op == OP_ADC || ins->op == OP_CMP || ins->op == OP_SBC ||
           ins->op == OP_LDA || ins->op == OP_STA || ins->op == OP_LSR || ins->op == OP_ASL ||
           ins->op == OP_ROR || ins->op == OP_ROL || ins->op == OP_INC || ins->op == OP_DEC ||
           ins->op == OP_IGN);
      if (print_address_value)
      {
        const uint8_t offset = read_8b_op(encoding);
        const uint8_t zero_page_x_offset = cpu_make_zero_page_x_offset(cpu, offset);
        snprintf(buffer, sizeof buffer, "%s $%02X,X @ %02X = %02X", op_name, offset,
                 zero_page_x_offset, cpu_read_zero_page_x(cpu, offset));
      }
      else
      {
        snprintf(buffer, sizeof buffer, "%s $%02X,X", op_name, encoding & 0xff);
      }
    }
    break;
    case AM_ZERO_PAGE_Y:
    {
      const bool print_address_value =
          layout == IL_NINTENDULATOR && (ins->op == OP_LDX || ins->op == OP_STX);
      if (print_address_value)
      {
        const uint8_t offset = read_8b_op(encoding);
        const uint8_t zero_page_y_offset = cpu_make_zero_page_y_offset(cpu, offset);
        snprintf(buffer, sizeof buffer, "%s $%02X,Y @ %02X = %02X", op_name, offset,
                 zero_page_y_offset, cpu_read_zero_page_y(cpu, offset));
      }
      else
      {
        snprintf(buffer, sizeof buffer, "%s $%02X,Y", op_name, encoding & 0xff);
      }
    }
    break;
  }

  return buffer;
}
