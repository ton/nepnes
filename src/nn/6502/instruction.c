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
    [OP_TYA] = "TYA"};

static const struct Instruction instructions[256] = {
    {0x00, OP_BRK, 1, AM_IMPLIED, 7},
    {0x01, OP_ORA, 2, AM_INDIRECT_X, 6},
    {0},
    {0},
    {0},
    {0x05, OP_ORA, 2, AM_ZERO_PAGE, 3},
    {0x06, OP_ASL, 2, AM_ZERO_PAGE, 5},
    {0},
    {0x08, OP_PHP, 1, AM_IMPLIED, 3},
    {0x09, OP_ORA, 2, AM_IMMEDIATE, 2},
    {0x0A, OP_ASL, 1, AM_ACCUMULATOR, 2},
    {0},
    {0},
    {0x0D, OP_ORA, 3, AM_ABSOLUTE, 4},
    {0x0E, OP_ASL, 3, AM_ABSOLUTE, 6},
    {0},
    {0x10, OP_BPL, 2, AM_RELATIVE, 2 /* +1 on branch, +2 on page cross */},
    {0x11, OP_ORA, 2, AM_INDIRECT_Y, 5 /* +1 on page cross */},
    {0},
    {0},
    {0},
    {0x15, OP_ORA, 2, AM_ZERO_PAGE_X, 4},
    {0x16, OP_ASL, 2, AM_ZERO_PAGE_X, 6},
    {0},
    {0x18, OP_CLC, 1, AM_IMPLIED, 2},
    {0x19, OP_ORA, 3, AM_ABSOLUTE_Y, 4 /* +1 on page cross */},
    {0},
    {0},
    {0},
    {0x1D, OP_ORA, 3, AM_ABSOLUTE_X, 4 /* +1 on page cross */},
    {0x1E, OP_ASL, 3, AM_ABSOLUTE_X, 7},
    {0},
    {0x20, OP_JSR, 3, AM_ABSOLUTE, 6},
    {0x21, OP_AND, 2, AM_INDIRECT_X, 6},
    {0},
    {0},
    {0x24, OP_BIT, 2, AM_ZERO_PAGE, 3},
    {0x25, OP_AND, 2, AM_ZERO_PAGE, 3},
    {0x26, OP_ROL, 2, AM_ZERO_PAGE, 5},
    {0},
    {0x28, OP_PLP, 1, AM_IMPLIED, 4},
    {0x29, OP_AND, 2, AM_IMMEDIATE, 2},
    {0x2A, OP_ROL, 1, AM_ACCUMULATOR, 2},
    {0},
    {0x2C, OP_BIT, 3, AM_ABSOLUTE, 4},
    {0x2D, OP_AND, 3, AM_ABSOLUTE, 4},
    {0x2E, OP_ROL, 3, AM_ABSOLUTE, 6},
    {0},
    {0x30, OP_BMI, 2, AM_RELATIVE, 2 /* +1 on branch, +2 on page cross */},
    {0x31, OP_AND, 2, AM_INDIRECT_Y, 5 /* +1 on page cross */},
    {0},
    {0},
    {0},
    {0x35, OP_AND, 2, AM_ZERO_PAGE_X, 4},
    {0x36, OP_ROL, 2, AM_ZERO_PAGE_X, 6},
    {0},
    {0x38, OP_SEC, 1, AM_IMPLIED, 2},
    {0x39, OP_AND, 3, AM_ABSOLUTE_Y, 4 /* +1 on page cross */},
    {0},
    {0},
    {0},
    {0x3D, OP_AND, 3, AM_ABSOLUTE_X, 4 /* +1 on page cross */},
    {0x3E, OP_ROL, 3, AM_ABSOLUTE_X, 7},
    {0},
    {0x40, OP_RTI, 1, AM_IMPLIED, 6},
    {0x41, OP_EOR, 2, AM_INDIRECT_X, 6},
    {0},
    {0},
    {0},
    {0x45, OP_EOR, 2, AM_ZERO_PAGE, 3},
    {0x46, OP_LSR, 2, AM_ZERO_PAGE, 5},
    {0},
    {0x48, OP_PHA, 1, AM_IMPLIED, 3},
    {0x49, OP_EOR, 2, AM_IMMEDIATE, 2},
    {0x4A, OP_LSR, 1, AM_ACCUMULATOR, 2},
    {0},
    {0x4C, OP_JMP, 3, AM_ABSOLUTE, 3},
    {0x4D, OP_EOR, 3, AM_ABSOLUTE, 4},
    {0x4E, OP_LSR, 3, AM_ABSOLUTE, 6},
    {0},
    {0x50, OP_BVC, 2, AM_RELATIVE, 2 /* +1 on branch, +2 on page cross */},
    {0x51, OP_EOR, 2, AM_INDIRECT_Y, 5 /* +1 on page cross */},
    {0},
    {0},
    {0},
    {0x55, OP_EOR, 2, AM_ZERO_PAGE_X, 4},
    {0x56, OP_LSR, 2, AM_ZERO_PAGE_X, 6},
    {0},
    {0x58, OP_CLI, 1, AM_IMPLIED, 2},
    {0x59, OP_EOR, 3, AM_ABSOLUTE_Y, 4 /* +1 on page cross */},
    {0},
    {0},
    {0},
    {0x5D, OP_EOR, 3, AM_ABSOLUTE_X, 4 /* +1 on page cross */},
    {0x5E, OP_LSR, 3, AM_ABSOLUTE_X, 7},
    {0},
    {0x60, OP_RTS, 1, AM_IMPLIED, 6},
    {0x61, OP_ADC, 2, AM_INDIRECT_X, 6},
    {0},
    {0},
    {0},
    {0x65, OP_ADC, 2, AM_ZERO_PAGE, 3},
    {0x66, OP_ROR, 2, AM_ZERO_PAGE, 5},
    {0},
    {0x68, OP_PLA, 1, AM_IMPLIED, 4},
    {0x69, OP_ADC, 2, AM_IMMEDIATE, 2},
    {0x6A, OP_ROR, 1, AM_ACCUMULATOR, 2},
    {0},
    {0x6C, OP_JMP, 3, AM_INDIRECT, 5},
    {0x6D, OP_ADC, 3, AM_ABSOLUTE, 4},
    {0x6E, OP_ROR, 3, AM_ABSOLUTE, 6},
    {0},
    {0x70, OP_BVS, 2, AM_RELATIVE, 2 /* +1 on branch, +2 on page cross */},
    {0x71, OP_ADC, 2, AM_INDIRECT_Y, 5 /* +1 on page cross */},
    {0},
    {0},
    {0},
    {0x75, OP_ADC, 2, AM_ZERO_PAGE_X, 4},
    {0x76, OP_ROR, 2, AM_ZERO_PAGE_X, 6},
    {0},
    {0x78, OP_SEI, 1, AM_IMPLIED, 2},
    {0x79, OP_ADC, 3, AM_ABSOLUTE_Y, 4 /* +1 on page cross */},
    {0},
    {0},
    {0},
    {0x7D, OP_ADC, 3, AM_ABSOLUTE_X, 4 /* +1 on page cross */},
    {0x7E, OP_ROR, 3, AM_ABSOLUTE_X, 7},
    {0},
    {0},  // 80
    {0x81, OP_STA, 2, AM_INDIRECT_X, 6},
    {0},
    {0},
    {0x84, OP_STY, 2, AM_ZERO_PAGE, 3},
    {0x85, OP_STA, 2, AM_ZERO_PAGE, 3},
    {0x86, OP_STX, 2, AM_ZERO_PAGE, 3},
    {0},
    {0x88, OP_DEY, 1, AM_IMPLIED, 2},
    {0},
    {0x8A, OP_TXA, 1, AM_IMPLIED, 2},
    {0},
    {0x8C, OP_STY, 3, AM_ABSOLUTE, 4},
    {0x8D, OP_STA, 3, AM_ABSOLUTE, 4},
    {0x8E, OP_STX, 3, AM_ABSOLUTE, 4},
    {0},
    {0x90, OP_BCC, 2, AM_RELATIVE, 2 /* +1 on branch, +2 on page cross */},
    {0x91, OP_STA, 2, AM_INDIRECT_Y, 6},
    {0},
    {0},
    {0x94, OP_STY, 2, AM_ZERO_PAGE_X, 4},
    {0x95, OP_STA, 2, AM_ZERO_PAGE_X, 4},
    {0x96, OP_STX, 2, AM_ZERO_PAGE_Y, 4},
    {0},
    {0x98, OP_TYA, 1, AM_IMPLIED, 2},
    {0x99, OP_STA, 3, AM_ABSOLUTE_Y, 5},
    {0x9A, OP_TXS, 1, AM_IMPLIED, 2},
    {0},
    {0},
    {0x9D, OP_STA, 3, AM_ABSOLUTE_X, 5},
    {0},
    {0},
    {0xA0, OP_LDY, 2, AM_IMMEDIATE, 2},
    {0xA1, OP_LDA, 2, AM_INDIRECT_X, 6},
    {0xA2, OP_LDX, 2, AM_IMMEDIATE, 2},
    {0},
    {0xA4, OP_LDY, 2, AM_ZERO_PAGE, 3},
    {0xA5, OP_LDA, 2, AM_ZERO_PAGE, 3},
    {0xA6, OP_LDX, 2, AM_ZERO_PAGE, 3},
    {0},
    {0xA8, OP_TAY, 1, AM_IMPLIED, 2},
    {0xA9, OP_LDA, 2, AM_IMMEDIATE, 2},
    {0xAA, OP_TAX, 1, AM_IMPLIED, 2},
    {0},
    {0xAC, OP_LDY, 3, AM_ABSOLUTE, 4},
    {0xAD, OP_LDA, 3, AM_ABSOLUTE, 4},
    {0xAE, OP_LDX, 3, AM_ABSOLUTE, 4},
    {0},
    {0xB0, OP_BCS, 2, AM_RELATIVE, 2 /* +1 on branch, +2 on page cross */},
    {0xB1, OP_LDA, 2, AM_INDIRECT_Y, 5 /* +1 on page cross */},
    {0},
    {0},
    {0xB4, OP_LDY, 2, AM_ZERO_PAGE_X, 4},
    {0xB5, OP_LDA, 2, AM_ZERO_PAGE_X, 4},
    {0xB6, OP_LDX, 2, AM_ZERO_PAGE_Y, 4},
    {0},
    {0xB8, OP_CLV, 1, AM_IMPLIED, 2},
    {0xB9, OP_LDA, 3, AM_ABSOLUTE_Y, 4 /* +1 on page cross */},
    {0xBA, OP_TSX, 1, AM_IMPLIED, 2},
    {0},
    {0xBC, OP_LDY, 3, AM_ABSOLUTE_X, 4 /* +1 on page cross */},
    {0xBD, OP_LDA, 3, AM_ABSOLUTE_X, 4 /* +1 on page cross */},
    {0xBE, OP_LDX, 3, AM_ABSOLUTE_Y, 4 /* +1 on page cross */},
    {0},
    {0xC0, OP_CPY, 2, AM_IMMEDIATE, 2},
    {0xC1, OP_CMP, 2, AM_INDIRECT_X, 6},
    {0},
    {0},
    {0xC4, OP_CPY, 2, AM_ZERO_PAGE, 3},
    {0xC5, OP_CMP, 2, AM_ZERO_PAGE, 3},
    {0xC6, OP_DEC, 2, AM_ZERO_PAGE, 5},
    {0},
    {0xC8, OP_INY, 1, AM_IMPLIED, 2},
    {0xC9, OP_CMP, 2, AM_IMMEDIATE, 2},
    {0xCA, OP_DEX, 1, AM_IMPLIED, 2},
    {0},
    {0xCC, OP_CPY, 3, AM_ABSOLUTE, 4},
    {0xCD, OP_CMP, 3, AM_ABSOLUTE, 4},
    {0xCE, OP_DEC, 3, AM_ABSOLUTE, 6},
    {0},
    {0xD0, OP_BNE, 2, AM_RELATIVE, 2 /* +1 on branch, +2 on page cross */},
    {0xD1, OP_CMP, 2, AM_INDIRECT_Y, 5 /* +1 on page cross */},
    {0},
    {0},
    {0},
    {0xD5, OP_CMP, 2, AM_ZERO_PAGE_X, 4},
    {0xD6, OP_DEC, 2, AM_ZERO_PAGE_X, 6},
    {0},
    {0xD8, OP_CLD, 1, AM_IMPLIED, 2},
    {0xD9, OP_CMP, 3, AM_ABSOLUTE_Y, 4 /* +1 on page cross */},
    {0},
    {0},
    {0},
    {0xDD, OP_CMP, 3, AM_ABSOLUTE_X, 4 /* +1 on page cross */},
    {0xDE, OP_DEC, 3, AM_ABSOLUTE_X, 7},
    {0},
    {0xE0, OP_CPX, 2, AM_IMMEDIATE, 2},
    {0xE1, OP_SBC, 2, AM_INDIRECT_X, 6},
    {0},
    {0},
    {0xE4, OP_CPX, 2, AM_ZERO_PAGE, 3},
    {0xE5, OP_SBC, 2, AM_ZERO_PAGE, 3},
    {0xE6, OP_INC, 2, AM_ZERO_PAGE, 5},
    {0},
    {0xE8, OP_INX, 1, AM_IMPLIED, 2},
    {0xE9, OP_SBC, 2, AM_IMMEDIATE, 2},
    {0xEA, OP_NOP, 1, AM_IMPLIED, 2},
    {0},
    {0xEC, OP_CPX, 3, AM_ABSOLUTE, 4},
    {0xED, OP_SBC, 3, AM_ABSOLUTE, 4},
    {0xEE, OP_INC, 3, AM_ABSOLUTE, 6},
    {0},
    {0xF0, OP_BEQ, 2, AM_RELATIVE, 2 /* +1 on branch, +2 on page cross */},
    {0xF1, OP_SBC, 2, AM_INDIRECT_Y, 5 /* +1 on page cross */},
    {0},
    {0},
    {0},
    {0xF5, OP_SBC, 2, AM_ZERO_PAGE_X, 4},
    {0xF6, OP_INC, 2, AM_ZERO_PAGE_X, 6},
    {0},
    {0xF8, OP_SED, 1, AM_IMPLIED, 2},
    {0xF9, OP_SBC, 3, AM_ABSOLUTE_Y, 4 /* +1 on page cross */},
    {0},
    {0},
    {0},
    {0xFD, OP_SBC, 3, AM_ABSOLUTE_X, 4 /* +1 on page cross */},
    {0xFE, OP_INC, 3, AM_ABSOLUTE_X, 7},
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
           ins->op == OP_DEC);
      if (print_address_value)
      {
        snprintf(buffer, sizeof buffer, "%s $%04X = %02X", operation_name(ins->op),
                 read_16b_op(encoding), cpu->ram[read_16b_op(encoding)]);
      }
      else
      {
        snprintf(buffer, sizeof buffer, "%s $%04X", operation_name(ins->op), read_16b_op(encoding));
      }
    }
    break;
    case AM_ABSOLUTE_X:
      snprintf(buffer, sizeof buffer, "%s $%04X,X", operation_name(ins->op), read_16b_op(encoding));
      break;
    case AM_ABSOLUTE_Y:
      snprintf(buffer, sizeof buffer, "%s $%04X,Y", operation_name(ins->op), read_16b_op(encoding));
      break;
    case AM_ACCUMULATOR:
      snprintf(buffer, sizeof buffer, "%s A", operation_name(ins->op));
      break;
    case AM_IMMEDIATE:
      snprintf(buffer, sizeof buffer, "%s #$%02X", operation_name(ins->op), read_8b_op(encoding));
      break;
    case AM_IMPLIED:
      snprintf(buffer, sizeof buffer, "%s", operation_name(ins->op));
      break;
    case AM_INDIRECT:
      snprintf(buffer, sizeof buffer, "%s ($%04X)", operation_name(ins->op), read_16b_op(encoding));
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

        snprintf(buffer, sizeof buffer, "%s ($%02X,X) @ %02X = %04X = %02X",
                 operation_name(ins->op), operand, (uint8_t)(operand + cpu->X), ptr, data);
      }
      else
      {
        snprintf(buffer, sizeof buffer, "%s ($%02X,X)", operation_name(ins->op), operand);
      }
    }
    break;
    case AM_INDIRECT_Y:
      snprintf(buffer, sizeof buffer, "%s ($%02X),Y", operation_name(ins->op),
               read_8b_op(encoding));
      break;
    case AM_RELATIVE:
      switch (layout)
      {
        case IL_NES_DISASM:
          snprintf(buffer, sizeof buffer, "%s $%02X (%d)", operation_name(ins->op),
                   read_8b_op(encoding), read_8b_op(encoding));
          break;
        case IL_NINTENDULATOR:
          snprintf(buffer, sizeof buffer, "%s $%02X", operation_name(ins->op),
                   cpu->PC + ins->bytes + (read_8b_op(encoding)));
          break;
      }
      break;
    case AM_ZERO_PAGE:
      switch (layout)
      {
        case IL_NES_DISASM:
          snprintf(buffer, sizeof buffer, "%s $%02X", operation_name(ins->op),
                   read_8b_op(encoding));
          break;
        case IL_NINTENDULATOR:
          snprintf(buffer, sizeof buffer, "%s $%02X = %02X", operation_name(ins->op),
                   read_8b_op(encoding), cpu->ram[read_8b_op(encoding)]);
          break;
      }
      break;
    case AM_ZERO_PAGE_X:
      snprintf(buffer, sizeof buffer, "%s $%02X,X", operation_name(ins->op), encoding & 0xff);
      break;
    case AM_ZERO_PAGE_Y:
      snprintf(buffer, sizeof buffer, "%s $%02X,Y", operation_name(ins->op), encoding & 0xff);
      break;
  }

  return buffer;
}
