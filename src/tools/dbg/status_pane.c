#include "status_pane.h"

#include "cpu.h"
#include "debugger.h"
#include "instruction.h"
#include "nc.h"
#include "util.h"
#include "version.h"

#include <notcurses/notcurses.h>

/*
 * Constructs the plane that contains a view on the memory of the machine, where
 * the values in memory are interpreted as instructions. This will also draw a
 * rounded border around the assembly plane on the standard notcurses plane.
 */
struct status_pane make_status_pane(struct ncplane *std_plane)
{
  struct status_pane pane;

  const int cols = ncplane_dim_x(std_plane);
  const int rows = 1;

  pane.plane = nn_make_simple_plane(std_plane, ncplane_dim_y(std_plane) - 1, 0, rows, cols);

  nccell c = CELL_CHAR_INITIALIZER(' ');
  cell_set_bg_rgb8(&c, 0x20, 0x20, 0x20);
  ncplane_set_base_cell(pane.plane, &c);

  return pane;
}

/*
 * Prints a help screen in the status line.
 */
void status_pane_print_help(struct status_pane *pane)
{
  ncplane_erase(pane->plane);
  ncplane_putstr_aligned(pane->plane, 0, NCALIGN_LEFT,
                         " j/k: scroll up/down   "
                         "C-B/C-F: page up/down   "
                         "f: focus PC   "
                         "r: run    "
                         "c: break at cycle   "
                         "n: next instruction   "
                         "q: quit");
  ncplane_putstr_aligned(pane->plane, 0, NCALIGN_RIGHT, "press any key to close help ");
}

/*
 * Prints all relevant CPU state data in the given CPU pane.
 */
void status_pane_update(struct status_pane *pane)
{
  ncplane_putstr_aligned(pane->plane, 0, NCALIGN_RIGHT, "nepnes dbg v" VERSION " ");
  ncplane_putstr_aligned(pane->plane, 0, NCALIGN_LEFT, " ?: help");
}

/*
 * Updates the status pane after resizing the terminal window.
 */
void status_pane_resize(struct status_pane *pane, const int term_rows, const int term_cols)
{
  ncplane_erase(pane->plane);
  ncplane_resize_simple(pane->plane, 1, term_cols);
  ncplane_move_yx(pane->plane, term_rows - 1, 0);
}
