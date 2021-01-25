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

#define CPU_MAX_ADDRESS 0xffff

enum Flags
{
  FLAGS_NONE = 0x00,
  FLAGS_CARRY = 0x01,
  FLAGS_ZERO = 0x02,
  FLAGS_INTERRUPT_DISABLE = 0x04,
  FLAGS_DECIMAL = 0x08,
  FLAGS_OVERFLOW = 0x40,
  FLAGS_NEGATIVE = 0x80,
};

/*
 * Representation of the 6502 CPU.
 */
struct Cpu
{
  uint8_t A;   /* Accumulator */
  uint8_t X;   /* X index register */
  uint8_t Y;   /* Y index register */
  uint8_t S;   /* Stack Pointer */
  uint8_t P;   /* Status register */
  uint16_t PC; /* Program Counter */

  uint8_t ram[CPU_MAX_ADDRESS + 1];

  unsigned cycle; /* Number of cycles elapsed since execution */
};

typedef uint16_t Address;

void cpu_execute_next_instruction(struct Cpu *cpu);

void cpu_power_on(struct Cpu *cpu);
void cpu_reset(struct Cpu *cpu);

#endif
