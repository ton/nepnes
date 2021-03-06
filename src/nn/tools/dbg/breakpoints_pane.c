#include "breakpoints_pane.h"

#include "debugger.h"
#include "nc.h"

#include <nn/6502/cpu.h>
#include <nn/6502/instruction.h>
#include <nn/std/util.h>

#include <notcurses/notcurses.h>

/*
 * Draws the border next to the assembly plane. Also called in case of a resize
 * event.
 */
static void breakpoints_plane_draw_border(struct ncplane *breakpoints_plane,
                                          struct ncplane *std_plane, int lines)
{
  nccell vline_cell = CELL_TRIVIAL_INITIALIZER;
  cell_load(std_plane, &vline_cell, "\u2502");
  ncplane_cursor_move_yx(std_plane, 0, ncplane_dim_x(breakpoints_plane));
  ncplane_vline(std_plane, &vline_cell, lines);
}

/*
 * Constructs the plane that contains a view on the memory of the machine, where
 * the values in memory are interpreted as instructions. This will also draw a
 * rounded border around the assembly plane on the standard notcurses plane.
 */
struct breakpoints_pane make_breakpoints_pane(struct ncplane *std_plane, struct debugger *debugger,
                                              const int rows, const int cols)
{
  struct breakpoints_pane pane;

  pane.debugger = debugger;
  pane.first = 0;
  pane.cursor_offset = 0;

  pane.decoration_plane = nn_make_pane_plane(std_plane, "Breakpoints", 0, 0, rows, cols);
  pane.contents_plane = nn_make_simple_plane(pane.decoration_plane, 1, 1, rows - 2, cols - 2);
  pane.cursor_plane = nn_make_line_plane(pane.contents_plane, 0xff, 0xff, 0xff, 0x44, 0x44, 0x44);
  ncplane_printf_yx(pane.cursor_plane, 0, 0, ">");

  breakpoints_pane_set_focus(&pane, false);

  return pane;
}

/*
 * Prints the assembly currently loaded in memory to the given notcurses plane.
 * Highlights the program counter, in case it is in view, draws the cursor line,
 * and highlights any breakpoints that are in view.
 */
void breakpoints_pane_update(struct breakpoints_pane *pane, struct debugger *debugger,
                             struct cpu *cpu)
{
  struct ncplane *plane = pane->contents_plane;
  ncplane_erase(plane);

  const unsigned lines = ncplane_dim_y(plane);
  const size_t n = MIN(debugger->breakpoints.size, pane->first + lines);
  unsigned y = 0;
  for (size_t i = pane->first; i < n; ++i, ++y)
  {
    ncplane_printf_yx(plane, y, 1, "$%04X: unconditional", debugger->breakpoints.data[i]);
  }
}

/*
 * Redraws the assembly pane after a resize.
 */
void breakpoints_pane_resize(struct breakpoints_pane *pane, struct ncplane *std_plane,
                             const int lines)
{
  /* breakpoints_plane_draw_border(pane->plane, std_plane, lines); */
}

/*
 * Moves the cursor in the assembly view by the given number of lines.
 */
void breakpoints_pane_move_cursor(struct breakpoints_pane *pane, int offset)
{
  pane->cursor_offset += offset;

  /* In case the cursor offset is outside the bounds of the current pane, scroll
   * the view. */
  if (pane->cursor_offset < 0)
  {
    pane->first = MAX(0, pane->first + offset);
    pane->cursor_offset = 0;
  }
  else if (offset > 0)
  {
    const int last_breakpoint_offset = (int)pane->debugger->breakpoints.size - 1;
    if (pane->first + pane->cursor_offset > last_breakpoint_offset)
    {
      pane->cursor_offset = last_breakpoint_offset - pane->first;
    }

    const int last_cursor_offset = ncplane_dim_y(pane->contents_plane) - 1;
    if (pane->cursor_offset > last_cursor_offset)
    {
      pane->first = pane->first + offset;
      pane->cursor_offset = last_cursor_offset;
    }
  }

  /* Position the cursor plane (if this pane has focus). */
  if (pane->has_focus)
  {
    ncplane_move_yx(pane->cursor_plane, pane->cursor_offset, 0);
  }
}

/*
 * Either sets or removes focus for this pane. In case the pane has focus, the
 * cursor is visible, otherwise it is hidden.
 */
void breakpoints_pane_set_focus(struct breakpoints_pane *pane, bool has_focus)
{
  if ((pane->has_focus = has_focus))
  {
    ncplane_move_yx(pane->cursor_plane, pane->cursor_offset, 0);
  }
  else
  {
    ncplane_move_yx(pane->cursor_plane, -2, 0);
  }
}

/*
 * Returns the index of the currently selected breakpoint, or -1 in case no
 * breakpoint is selected.
 */
int breakpoints_pane_selected_breakpoint(struct breakpoints_pane *pane)
{
  return pane->has_focus ? pane->first + pane->cursor_offset : -1;
}
