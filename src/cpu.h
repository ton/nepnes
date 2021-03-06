#ifndef NEPNES_CPU_H
#define NEPNES_CPU_H

#include "nn.h"

#include <stddef.h>
#include <stdint.h>

/*
 * The NES CPU core is based on the 6502 processor and runs at approximately
 * 1.79 MHz (1.66 MHz in a PAL NES). It is made by Ricoh and lacks the MOS6502's
 * decimal mode. In the NTSC NES, the RP2A03 chip contains the CPU and APU; in
 * the PAL NES, the CPU and APU are contained within the RP2A07 chip.
 */

#define CPU_MAX_ADDRESS 0xffff

/*
 * Enumeration of the flag values.
 */
enum Flags
{
  FLAGS_NONE = 0x00,
  FLAGS_CARRY = 0x01,
  FLAGS_ZERO = 0x02,
  FLAGS_INTERRUPT_DISABLE = 0x04,
  FLAGS_DECIMAL = 0x08,
  FLAGS_BIT_4 = 0x10,
  FLAGS_BIT_5 = 0x20,
  FLAGS_OVERFLOW = 0x40,
  FLAGS_NEGATIVE = 0x80,

  FLAGS_BRK_PHP_PUSH = 0x30, /* bit 4 and 5 are set in case the flags are pushed
                                because of PHP/BRK */
};

/*
 * Enumeration of the bit indexes of the various flags.
 */
enum FlagsBit
{
  FLAGS_BIT_CARRY = 0,
  FLAGS_BIT_ZERO = 1,
  FLAGS_BIT_INTERRUPT_DISABLE = 2,
  FLAGS_BIT_DECIMAL = 3,
  FLAGS_BIT_BIT_4 = 4,
  FLAGS_BIT_BIT_5 = 5,
  FLAGS_BIT_OVERFLOW = 6,
  FLAGS_BIT_NEGATIVE = 7
};

/*
 * Representation of the 6502 CPU.
 */
struct cpu
{
  uint8_t A;  /* Accumulator */
  uint8_t X;  /* X index register */
  uint8_t Y;  /* Y index register */
  uint8_t S;  /* Stack Pointer */
  uint8_t P;  /* Status register */
  Address PC; /* Program Counter */

  uint8_t ram[CPU_MAX_ADDRESS + 1];

  unsigned cycle; /* Number of cycles elapsed since execution */
};

void cpu_execute_next_instruction(struct cpu *cpu);

void cpu_power_on(struct cpu *cpu);
void cpu_reset(struct cpu *cpu);

#endif
