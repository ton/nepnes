#ifndef NEPNES_CPU_H
#define NEPNES_CPU_H

#include <stdint.h>

/*
 * The NES CPU core is based on the 6502 processor and runs at approximately
 * 1.79 MHz (1.66 MHz in a PAL NES). It is made by Ricoh and lacks the MOS6502's
 * decimal mode. In the NTSC NES, the RP2A03 chip contains the CPU and APU; in
 * the PAL NES, the CPU and APU are contained within the RP2A07 chip.
 */

enum AddressingMode
{
  /*
   * Indexed indirect addressing is normally used in conjunction with a table of
   * address held on zero page. The address of the table is taken from the
   * instruction and the X register added to it (with zero page wrap around) to
   * give the location of the least significant byte of the target address.
   */
  AddressingMode_IndexedIndirect,
};

/* Enumeration of all possible instructions supported by the NES. */
enum Instruction
{
  And
};

struct Opcode
{
  uint8_t opcode;
  enum AddressingMode addressing;
  int bytes;
  int cycles;
};

enum Instruction make_instruction(uint8_t opcode);
struct Opcode make_opcode(uint8_t opcode);

#endif
