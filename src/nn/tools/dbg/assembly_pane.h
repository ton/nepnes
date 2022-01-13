#ifndef NEPNES_ASSEMBLY_PANE_H
#define NEPNES_ASSEMBLY_PANE_H

#include <nn/nn.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct ncplane;

struct Cpu;
struct Debugger;

/* Stores state of the assembly pane. This deals primarily with presentation
 * logic, using notcurses as the back end. */
struct AssemblyPane
{
  const struct Debugger *debugger; /* non-owning pointer to the debugger state */
  const struct Cpu *cpu;           /* non-owning pointer to the CPU state */

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

struct AssemblyPane make_assembly_pane(struct ncplane *std_plane, struct Debugger *debugger,
                                       struct Cpu *cpu, const int lines, const int cols,
                                       const int y, const int x);
void destroy_assembly_pane(struct AssemblyPane *pane);

void assembly_pane_update(struct AssemblyPane *pane);
void assembly_pane_resize(struct AssemblyPane *pane, struct ncplane *std_plane, const int lines);

Address assembly_pane_cursor_address(struct AssemblyPane *pane, struct Debugger *debugger,
                                     struct Cpu *cpu);
void assembly_pane_move_cursor(struct AssemblyPane *pane, int offset);
void assembly_pane_scroll_to_address(struct AssemblyPane *pane, struct Debugger *debugger,
                                     struct Cpu *cpu, Address address);
void assembly_pane_scroll_to_pc(struct AssemblyPane *pane, struct Debugger *debugger,
                                struct Cpu *cpu);

void assembly_pane_set_focus(struct AssemblyPane *pane, bool has_focus);

#endif
