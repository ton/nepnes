#ifndef NEPNES_APP_DBG_CPU_PANE_H
#define NEPNES_APP_DBG_CPU_PANE_H

#include <stdbool.h>
#include <stdint.h>

struct ncplane;

struct Cpu;

/* Stores state of the assembly pane. This deals primarily with presentation
 * logic, using notcurses as the back end. */
struct CpuPane
{
  struct ncplane *decoration_plane; /* base plane, holds the window border and title */
  struct ncplane *contents_plane;   /* holds the list of breakpoints */
};

struct CpuPane make_cpu_pane(const int lines, const int cols, struct ncplane *std_plane);
void cpu_pane_update(struct CpuPane *pane, struct Cpu *cpu);

#endif
