#include "cpu.h"

/*
 * Constructs an opcode structure containing meta information on the given
 * opcode, like the number of bytes it occupies in memory, the number of cycles
 * it takes to execute, and the addressing mode it uses.
 */
struct Opcode make_opcode(uint8_t opcode)
{
  struct Opcode result = {0};

  switch (opcode)
  {
    case And:
      break;
  }

  return result;
}
