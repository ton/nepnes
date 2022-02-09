#ifndef NEPNES_STATUS_PANE_H
#define NEPNES_STATUS_PANE_H

#include <stdbool.h>
#include <stdint.h>

struct ncplane;

struct Cpu;

/* Stores state of the assembly pane. This deals primarily with presentation
 * logic, using notcurses as the back end. */
struct status_pane
{
  struct ncplane *plane; /* base plane, holds the status line character data */
};

struct status_pane make_status_pane(struct ncplane *std_plane);

void status_pane_print_help(struct status_pane *pane);
void status_pane_update(struct status_pane *pane);
void status_pane_resize(struct status_pane *pane, const int term_rows, const int term_cols);

#endif
