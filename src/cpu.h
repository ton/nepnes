#ifndef NEPNES_CPU_H
#define NEPNES_CPU_H

#include <stddef.h>
#include <stdint.h>

/*
 * The NES CPU core is based on the 6502 processor and runs at approximately
 * 1.79 MHz (1.66 MHz in a PAL NES). It is made by Ricoh and lacks the MOS6502's
 * decimal mode. In the NTSC NES, the RP2A03 chip contains the CPU and APU; in
 * the PAL NES, the CPU and APU are contained within the RP2A07 chip.
 */

/*
 * Representation of the 6502 CPU.
 */
struct Cpu
{
  uint8_t A;   /* Accumulator */
  uint8_t X;   /* X index register */
  uint8_t Y;   /* Y index register */
  uint8_t S;   /* Stack Pointer */
  uint16_t PC; /* Program Counter */

  uint8_t ram[0xffff + 1];
};

int cpu_instruction_count(struct Cpu *cpu, uint16_t offset);
uint16_t cpu_find_instruction_address(struct Cpu *cpu, int n);

#endif
