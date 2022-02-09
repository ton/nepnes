#ifndef NEPNES_APP_DBG_NC_H
#define NEPNES_APP_DBG_NC_H

#include <stdint.h>

struct ncplane;

struct ncplane *nn_make_line_plane(struct ncplane *breakpoints_plane, uint8_t fr, uint8_t fg,
                                   uint8_t fb, uint8_t br, uint8_t bg, uint8_t bb);
struct ncplane *nn_make_simple_plane(struct ncplane *parent_plane, int y, int x, int rows,
                                     int cols);
struct ncplane *nn_make_pane_plane(struct ncplane *parent_plane, const char *title, int y, int x,
                                   int rows, int cols);

#endif
