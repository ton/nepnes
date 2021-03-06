#ifndef NEPNES_ASSEMBLY_PANE_H
#define NEPNES_ASSEMBLY_PANE_H

#include <nn/nn.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct ncplane;

struct cpu;
struct debugger;

/* Stores state of the assembly pane. This deals primarily with presentation
 * logic, using notcurses as the back end. */
struct assembly_pane
{
  const struct debugger *debugger; /* non-owning pointer to the debugger state */
  const struct cpu *cpu;           /* non-owning pointer to the CPU state */

  Address first; /* first address to display */

  int first_offset;  /* offset of the first instruction to display */
  int last_offset;   /* offset of the last instruction or byte in memory */
  int cursor_offset; /* cursor offset (in instructions) from the first
                        address to display */
  int scroll_offset; /* minimal number of lines to keep above and below
                        the cursor line */

  bool has_focus; /* indicates whether this pane has focus (should display the cursor) */

  uint16_t address; /* address to focus */

  struct ncplane *plane;        /* holds the character data */
  struct ncplane *pc_plane;     /* highlights the program counter location */
  struct ncplane *cursor_plane; /* highlights the cursor location */

  struct ncplane **breakpoint_planes;
  size_t breakpoint_planes_size;
  size_t breakpoint_planes_capacity;
};

struct assembly_pane make_assembly_pane(struct ncplane *std_plane, struct debugger *debugger,
                                        struct cpu *cpu, const int lines, const int cols,
                                        const int y, const int x);
void destroy_assembly_pane(struct assembly_pane *pane);

void assembly_pane_update(struct assembly_pane *pane);
void assembly_pane_resize(struct assembly_pane *pane, struct ncplane *std_plane, const int lines);

Address assembly_pane_cursor_address(struct assembly_pane *pane, struct debugger *debugger,
                                     struct cpu *cpu);
void assembly_pane_move_cursor(struct assembly_pane *pane, int offset);
void assembly_pane_scroll_to_address(struct assembly_pane *pane, struct debugger *debugger,
                                     struct cpu *cpu, Address address);
void assembly_pane_scroll_to_pc(struct assembly_pane *pane, struct debugger *debugger,
                                struct cpu *cpu);

void assembly_pane_set_focus(struct assembly_pane *pane, bool has_focus);

#endif
