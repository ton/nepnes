#include "da.h"

#include "cpu.h"
#include "instruction.h"

/*
 * Disassembles the given ROM program data, with the given size, and outputs
 * assembly instructions to the given file pointer. Returns zero in case output
 * to the given file pointer succeeded, or the error code on the file pointer in
 * case output fails.
 */
int da_disassemble(FILE *fp, uint8_t *prg_data, size_t prg_size)
{
  /* Address where the NES typically stores PRG data. */
  const uint32_t prg_rom = 0x08000;

  uint32_t rom_offset = prg_rom;

  uint8_t *pc = prg_data;
  uint8_t *end = prg_data + prg_size;
  while (pc < end)
  {
    struct Instruction ins = make_instruction(*pc);

    /* In case of an unknown instruction, the calculated opcode of the
     * instruction will be zero. */
    if (ins.bytes == 0)
    {
      fprintf(fp, "$%X: %*s (%02X)\n", rom_offset, INSTRUCTION_BUFSIZE, "", *pc);

      ++pc;
      ++rom_offset;
    }
    else
    {
      uint32_t encoding = *pc;
      for (int i = 1; i < ins.bytes; ++i)
      {
        encoding = (encoding << 8) + (pc + i < end ? *(pc + i) : 0);
      }

      fprintf(fp, "$%X: %-*s (%0*X)\n", rom_offset, INSTRUCTION_BUFSIZE,
              instruction_print(&ins, encoding), ins.bytes * 2, encoding);

      pc += ins.bytes;
      rom_offset += ins.bytes;
    }
  }

  return ferror(fp);
}
