#include "assembly_pane.h"

#include "debugger.h"

#include <nn/6502/cpu.h>
#include <nn/6502/instruction.h>
#include <nn/std/util.h>

#include <notcurses/notcurses.h>

/*
 * Draws the border next to the assembly plane. Also called in case of a resize
 * event.
 */
static void assembly_plane_draw_border(struct ncplane *assembly_plane, struct ncplane *std_plane,
                                       int lines)
{
  nccell vline_cell = NCCELL_TRIVIAL_INITIALIZER;
  nccell_load(std_plane, &vline_cell, "\u2502");
  ncplane_cursor_move_yx(std_plane, 0, ncplane_dim_x(assembly_plane));
  ncplane_vline(std_plane, &vline_cell, lines);
}

/*
 * After the contents in the assembly pane scroll in a certain direction, this repositions the bars
 * that highlight certain instructions, like the instruction the program counter is pointing to,
 * breakpoints, etc. */
static void assembly_pane_update_line_planes(struct assembly_pane *pane, int delta_first_offset)
{
  /* Position the cursor plane (if this pane has focus). */
  if (pane->has_focus)
  {
    ncplane_move_yx(pane->cursor_plane, pane->cursor_offset, 0);
  }

  /* Reposition the PC plane. */
  ncplane_move_yx(pane->pc_plane, ncplane_y(pane->pc_plane) + delta_first_offset, 0);
}

/*
 * Creates a plane that will highlight a line in the assembly pane. The lines
 * foreground and background RGB triplets are set using (fr, fg, fb), and (br,
 * bg, bb) respectively.
 */
static struct ncplane *make_line_plane(struct ncplane *assembly_plane, uint8_t fr, uint8_t fg,
                                       uint8_t fb, uint8_t br, uint8_t bg, uint8_t bb)
{
  struct ncplane_options opts = {0};
  opts.rows = 1;
  opts.cols = ncplane_dim_x(assembly_plane);
  opts.y = 1;
  opts.x = 0;

  nccell c = NCCELL_TRIVIAL_INITIALIZER;
  nccell_set_fg_rgb8(&c, fr, fg, fb);
  nccell_set_bg_rgb8(&c, br, bg, bb);

  struct ncplane *plane = ncplane_create(assembly_plane, &opts);
  ncplane_set_base_cell(plane, &c);
  return plane;
}

/*
 * Constructs the plane that contains a view on the memory of the machine, where
 * the values in memory are interpreted as instructions. This will also draw a
 * rounded border around the assembly plane on the standard notcurses plane.
 */
struct assembly_pane make_assembly_pane(struct ncplane *std_plane, struct debugger *debugger,
                                        struct cpu *cpu, const int lines, const int cols,
                                        const int y, const int x)
{
  struct assembly_pane pane;

  pane.cpu = cpu;
  pane.debugger = debugger;

  /* Create the notcurses plane. */
  struct ncplane_options opts = {0};
  opts.x = x;
  opts.y = y;
  opts.rows = lines;
  opts.cols = cols;

  pane.plane = ncplane_create(std_plane, &opts);
  pane.pc_plane = make_line_plane(pane.plane, 0x44, 0x44, 0x44, 0xff, 0xff, 0xff);
  pane.cursor_plane = make_line_plane(pane.plane, 0xff, 0xff, 0xff, 0x44, 0x44, 0x44);
  ncplane_printf_yx(pane.cursor_plane, 0, 0, ">");

  ncplane_move_above(pane.pc_plane, pane.plane);
  ncplane_move_above(pane.cursor_plane, pane.plane);

  pane.first = 0x0;
  pane.first_offset = 0;
  /* TODO(ton): in case a program writes to PRG ROM mapped memory, this needs to
   * be recalculated! */
  pane.last_offset = debugger_address_to_instruction_offset(debugger, cpu, CPU_MAX_ADDRESS);
  pane.scroll_offset = 3;
  pane.has_focus = false;

  pane.breakpoint_planes_size = 0;
  pane.breakpoint_planes_capacity = 16;
  pane.breakpoint_planes = malloc(pane.breakpoint_planes_capacity * sizeof(struct ncplane *));

  /* Draw a vertical line separating the assembly window from the rest of the
   * UI. */
  assembly_plane_draw_border(pane.plane, std_plane, lines);

  return pane;
}

/*
 * Destroys all dynamically allocated memory associated with an assembly pane.
 * TODO(ton): Do we even need this...? Let the OS take care of it...
 */
void destroy_assembly_pane(struct assembly_pane *pane)
{
  free(pane->breakpoint_planes);
}

/*
 * Prints the assembly currently loaded in memory to the given notcurses plane.
 * Highlights the program counter, in case it is in view, draws the cursor line,
 * and highlights any breakpoints that are in view.
 */
void assembly_pane_update(struct assembly_pane *pane)
{
  uint32_t address = pane->first;
  const uint32_t prg_last_address = pane->debugger->prg_offset + pane->debugger->prg_size;

  /* Redraw all instructions. */
  /* TODO(ton): elide in case nothing changed since the last draw by keeping
   * some dirty flag on the memory contents. */
  ncplane_erase(pane->plane);

  int y = 0;
  const int lines = ncplane_dim_y(pane->plane);
  while (y < lines && address <= CPU_MAX_ADDRESS)
  {
    struct Instruction ins = {0};
    if (pane->debugger->prg_offset <= address && address < prg_last_address)
    {
      ins = make_instruction(pane->cpu->ram[address]);
    }

    /*
     * In case of an unknown instruction, the calculated size of the instruction
     * will be zero.
     */
    if (ins.bytes == 0)
    {
      ncplane_printf_yx(pane->plane, y, 1, "$%04X: %*s (%02X)", address, INSTRUCTION_BUFSIZE, "",
                        pane->cpu->ram[address]);
      ++address;
    }
    else
    {
      uint32_t encoding = pane->cpu->ram[address];
      for (int i = 1; i < ins.bytes; ++i)
      {
        encoding = (encoding << 8) + pane->cpu->ram[address + i];
      }

      ncplane_printf_yx(pane->plane, y, 1, "$%04X: %-*s (%0*X)", address, INSTRUCTION_BUFSIZE,
                        instruction_print(&ins, encoding), ins.bytes * 2, encoding);

      address += ins.bytes;
    }

    ++y;
  }

  /* Create breakpoint planes if needed. */
  if (pane->breakpoint_planes_size < pane->debugger->breakpoints.size)
  {
    /* Allocate additional memory if needed. */
    if (pane->breakpoint_planes_capacity < pane->debugger->breakpoints.size)
    {
      pane->breakpoint_planes_capacity = 2 * pane->debugger->breakpoints.size;
      pane->breakpoint_planes = realloc(
          pane->breakpoint_planes, pane->breakpoint_planes_capacity * sizeof(struct ncplane *));
    }

    for (size_t i = pane->breakpoint_planes_size; i < pane->debugger->breakpoints.size; ++i)
    {
      pane->breakpoint_planes[i] = make_line_plane(pane->plane, 0x44, 0x44, 0x44, 0xaa, 0x56, 0x78);
    }

    pane->breakpoint_planes_size = pane->debugger->breakpoints.size;
  }

  /* Position the breakpoints. */
  for (size_t i = 0; i < pane->debugger->breakpoints.size; ++i)
  {
    const int bp_offset = debugger_address_to_instruction_offset(
        pane->debugger, pane->cpu, pane->debugger->breakpoints.data[i]);
    struct ncplane *breakpoint_plane = pane->breakpoint_planes[i];
    ncplane_move_yx(breakpoint_plane, bp_offset - pane->first_offset, 0);
  }

  /* Hide all remaining breakpoints. */
  for (size_t i = pane->debugger->breakpoints.size; i < pane->breakpoint_planes_size; ++i)
  {
    struct ncplane *breakpoint_plane = pane->breakpoint_planes[i];
    ncplane_move_yx(breakpoint_plane, -2, 0);
  }
}

/*
 * Redraws the assembly pane after a resize.
 */
void assembly_pane_resize(struct assembly_pane *pane, struct ncplane *std_plane, const int lines)
{
  assembly_plane_draw_border(pane->plane, std_plane, lines);
}

/*
 * Returns the address where the cursor is positioned.
 */
Address assembly_pane_cursor_address(struct assembly_pane *pane, struct debugger *debugger,
                                     struct cpu *cpu)
{
  return debugger_instruction_offset_to_address(debugger, cpu,
                                                pane->first_offset + pane->cursor_offset);
}

/*
 * Moves the cursor in the assembly view by the given number of lines.
 */
void assembly_pane_move_cursor(struct assembly_pane *pane, int offset)
{
  pane->cursor_offset += offset;

  const int prev_first_offset = pane->first_offset;

  /* In case the cursor offset is outside the bounds of the current pane, scroll
   * the view. */
  if (pane->cursor_offset < 0)
  {
    pane->first_offset = MAX(0, pane->first_offset + pane->cursor_offset);
    pane->first =
        debugger_instruction_offset_to_address(pane->debugger, pane->cpu, pane->first_offset);
    pane->cursor_offset = 0;
  }
  else if (offset > 0)
  {
    if (pane->first_offset + pane->cursor_offset > pane->last_offset)
    {
      pane->cursor_offset = pane->last_offset - pane->first_offset;
    }

    /* Now, the cursor may be outside the current pane. In that case, first
     * scroll the view accordingly. */
    const int last_cursor_offset = ncplane_dim_y(pane->plane) - 1;
    if (pane->cursor_offset > last_cursor_offset)
    {
      pane->first_offset += pane->cursor_offset - last_cursor_offset;
      pane->first =
          debugger_instruction_offset_to_address(pane->debugger, pane->cpu, pane->first_offset);
      pane->cursor_offset = last_cursor_offset;
    }
  }

  assembly_pane_update_line_planes(pane, prev_first_offset - pane->first_offset);
}

/*
 * Scrolls the assembly pane to display the instruction that starts at or
 * overlaps the given address.
 */
void assembly_pane_scroll_to_address(struct assembly_pane *pane, struct debugger *debugger,
                                     struct cpu *cpu, Address address)
{
  const int prev_first_offset = pane->first_offset;

  pane->first = address;
  pane->first_offset = debugger_address_to_instruction_offset(debugger, cpu, address);
  pane->cursor_offset = 0;

  assembly_pane_update_line_planes(pane, prev_first_offset - pane->first_offset);
}

/*
 * Scrolls the assembly pane to display the instruction the program counter
 * points to. This will also update the location of the bar that highlights the
 * instruction at the address the program counter is pointing to.
 */
void assembly_pane_scroll_to_pc(struct assembly_pane *pane, struct debugger *debugger,
                                struct cpu *cpu)
{
  assembly_pane_scroll_to_address(pane, debugger, cpu, cpu->PC);

  /* Position the PC plane. */
  const int pc_offset = debugger_address_to_instruction_offset(debugger, cpu, cpu->PC);
  ncplane_move_yx(pane->pc_plane, pc_offset - pane->first_offset, 0);
}

/*
 * Either sets or removes focus for this pane. In case this pane has focus, the
 * cursor is visible, otherwise, it is hidden.
 */
void assembly_pane_set_focus(struct assembly_pane *pane, bool has_focus)
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
