#ifndef NEPNES_CPU_PANE_H
#define NEPNES_CPU_PANE_H

#include <nn/nn.h>

#include <stdbool.h>
#include <stdint.h>

struct ncplane;

struct cpu;

/* Stores state of the assembly pane. This deals primarily with presentation
 * logic, using notcurses as the back end. */
struct cpu_pane
{
  struct ncplane *decoration_plane; /* base plane, holds the window border and title */
  struct ncplane *contents_plane;   /* holds the list of breakpoints */
};

struct cpu_pane make_cpu_pane(const int lines, const int cols, struct ncplane *std_plane);
void cpu_pane_update(struct cpu_pane *pane, struct cpu *cpu);

#endif
