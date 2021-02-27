#include "nc.h"

#include <notcurses/notcurses.h>

/*
 * Creates a plane that will highlight a line in the assembly pane. The lines
 * foreground and background RGB triplets are set using (fr, fg, fb), and (br,
 * bg, bb) respectively.
 */
struct ncplane *nn_make_line_plane(struct ncplane *parent_plane, uint8_t fr, uint8_t fg, uint8_t fb,
                                   uint8_t br, uint8_t bg, uint8_t bb)
{
  struct ncplane_options opts = {0};
  opts.rows = 1;
  opts.cols = ncplane_dim_x(parent_plane);
  opts.y = 0;
  opts.x = 0;

  nccell c = CELL_TRIVIAL_INITIALIZER;
  cell_set_fg_rgb8(&c, fr, fg, fb);
  cell_set_bg_rgb8(&c, br, bg, bb);

  struct ncplane *plane = ncplane_create(parent_plane, &opts);
  ncplane_set_base_cell(plane, &c);
  return plane;
}

/*
 * Creates a plane at the given x- and y-coordinates, with the given dimensions.
 */
struct ncplane *nn_make_simple_plane(struct ncplane *parent_plane, int y, int x, int rows, int cols)
{
  struct ncplane_options opts = {0};
  opts.x = x;
  opts.y = y;
  opts.rows = rows;
  opts.cols = cols;

  return ncplane_create(parent_plane, &opts);
}

/*
 * Creates a plane that will represent a 'pane' in the UI, at the given x- and
 * y-coordinates, with the given dimensions. A pane has a border, and a title.
 */
struct ncplane *nn_make_pane_plane(struct ncplane *parent_plane, const char *title, int y, int x,
                                   int rows, int cols)
{
  struct ncplane *plane = nn_make_simple_plane(parent_plane, y, x, rows, cols);
  ncplane_perimeter_rounded(plane, 0, 0, 0);
  ncplane_printf_yx(plane, 0, 2, "%s %s %s", "\u257c", title, "\u257e");
  return plane;
}
