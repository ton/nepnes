#include "cpu_pane.h"

#include "cpu.h"
#include "dbg.h"
#include "instruction.h"
#include "nc.h"
#include "util.h"

#include <notcurses/notcurses.h>

/*
 * Constructs the plane that contains a view on the memory of the machine, where
 * the values in memory are interpreted as instructions. This will also draw a
 * rounded border around the assembly plane on the standard notcurses plane.
 */
struct cpu_pane make_cpu_pane(const int rows, const int cols, struct ncplane *std_plane)
{
  struct cpu_pane pane;

  pane.decoration_plane = nn_make_pane_plane(std_plane, "CPU", 0, 0, rows, cols);
  pane.contents_plane = nn_make_simple_plane(pane.decoration_plane, 1, 1, rows - 2, cols - 2);

  return pane;
}

/*
 * Prints all relevant CPU state data in the given CPU pane.
 */
void cpu_pane_update(struct cpu_pane *pane, struct Cpu *cpu)
{
  struct ncplane *plane = pane->contents_plane;
  ncplane_erase(plane);

  const char flags[] = {'-', 'C', 'Z', 'I', 'D', 'B', 'B', 'O', 'N'};

  ncplane_printf_yx(plane, 0, 1, "   A:  $%02X", cpu->A);
  ncplane_printf_yx(plane, 1, 1, "   X:  $%02X", cpu->X);
  ncplane_printf_yx(plane, 2, 1, "   Y:  $%02X", cpu->Y);
  ncplane_printf_yx(plane, 3, 1, "   S:  $%02X", cpu->S);
  ncplane_printf_yx(plane, 4, 1, "  PC:  $%04X", cpu->PC);
  ncplane_printf_yx(
      plane, 5, 1, "   P:  %c%c%c%c%c%c%c%c", flags[((cpu->P & FLAGS_NEGATIVE) >> 7) * 8],
      flags[((cpu->P & FLAGS_OVERFLOW) >> 6) * 7], flags[((cpu->P & FLAGS_BIT_5) >> 5) * 6],
      flags[((cpu->P & FLAGS_BIT_4) >> 4) * 5], flags[((cpu->P & FLAGS_DECIMAL) >> 3) * 4],
      flags[((cpu->P & FLAGS_INTERRUPT_DISABLE) >> 2) * 3], flags[(cpu->P & FLAGS_ZERO)],
      flags[(cpu->P & FLAGS_CARRY)]);
  ncplane_printf_yx(plane, 6, 1, " CYC:  %d", cpu->cycle);
}
