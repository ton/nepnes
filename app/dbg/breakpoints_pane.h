#ifndef NEPNES_BREAKPOINTS_PANE_H
#define NEPNES_BREAKPOINTS_PANE_H

#include "debugger.h"

#include <stdbool.h>
#include <stdint.h>

struct ncplane;

struct cpu;

/* Stores state of the assembly pane. This deals primarily with presentation
 * logic, using notcurses as the back end. */
struct BreakpointsPane
{
  const struct Debugger *debugger; /* non-owning pointer to the debugger state */

  int first;         /* first breakpoint entry to display */
  int cursor_offset; /* cursor offset (in breakpoints) from the first
                        breakpoint to display */

  bool has_focus;

  struct ncplane *decoration_plane; /* base plane, holds the window border and title */
  struct ncplane *contents_plane;   /* holds the list of breakpoints */
  struct ncplane *cursor_plane;     /* highlights the cursor location */
};

struct BreakpointsPane make_breakpoints_pane(struct ncplane *std_plane, struct Debugger *debugger,
                                             const int lines, const int cols);

void breakpoints_pane_update(struct BreakpointsPane *pane, struct Debugger *debugger);
// void breakpoints_pane_resize(struct BreakpointsPane *pane, struct ncplane *std_plane,
//                              const int lines);

void breakpoints_pane_move_cursor(struct BreakpointsPane *pane, int offset);
void breakpoints_pane_set_focus(struct BreakpointsPane *pane, bool has_focus);

int breakpoints_pane_selected_breakpoint(struct BreakpointsPane *pane);

#endif
