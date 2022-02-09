#ifndef NEPNES_6502_CPU_H
#define NEPNES_6502_CPU_H

#include <stddef.h>
#include <stdint.h>

/*
 * The NES CPU core is based on the 6502 processor and runs at approximately
 * 1.79 MHz (1.66 MHz in a PAL NES). It is made by Ricoh and lacks the MOS6502's
 * decimal mode. In the NTSC NES, the RP2A03 chip contains the CPU and APU; in
 * the PAL NES, the CPU and APU are contained within the RP2A07 chip.
 */

#define CPU_ADDRESS_RESET_VECTOR 0xfffc
#define CPU_ADDRESS_MAX 0xffff

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

typedef uint16_t Address;

/*
 * Representation of the 6502 CPU.
 */
struct Cpu
{
  uint8_t A;  /* Accumulator */
  uint8_t X;  /* X index register */
  uint8_t Y;  /* Y index register */
  uint8_t S;  /* Stack Pointer */
  uint8_t P;  /* Status register */
  Address PC; /* Program Counter */

  uint8_t ram[CPU_ADDRESS_MAX + 1];

  unsigned cycle; /* Number of cycles elapsed since execution */
};

uint8_t cpu_read_8b(struct Cpu *cpu, Address a);
int8_t cpu_read_signed_8b(struct Cpu *cpu, Address a);
uint16_t cpu_read_16b(struct Cpu *cpu, Address a);
uint16_t cpu_read_indirect_16b(struct Cpu *cpu, Address a);
void cpu_write_16b(struct Cpu *cpu, Address a, uint16_t x);

Address cpu_read_indirect_address(struct Cpu *cpu, uint8_t offset);
Address cpu_read_indirect_x_address(struct Cpu *cpu, uint8_t offset);
uint8_t cpu_read_indirect_x(struct Cpu *cpu, uint8_t offset);
Address cpu_read_indirect_y_address(struct Cpu *cpu, uint8_t offset);
uint8_t cpu_read_indirect_y(struct Cpu *cpu, uint8_t offset);

uint8_t cpu_make_zero_page_x_offset(struct Cpu *cpu, uint8_t offset);
uint8_t cpu_read_zero_page_x(struct Cpu *cpu, uint8_t offset);
uint8_t cpu_make_zero_page_y_offset(struct Cpu *cpu, uint8_t offset);
uint8_t cpu_read_zero_page_y(struct Cpu *cpu, uint8_t offset);

int cpu_page_cross(Address address, uint8_t offset);

void cpu_execute_next_instruction(struct Cpu *cpu);

void cpu_power_on(struct Cpu *cpu);
void cpu_reset(struct Cpu *cpu);

#endif
